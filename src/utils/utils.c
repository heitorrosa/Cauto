#include "../common/common.h"

#include "utils.h"

void initGlobalConfig(globalConfig *config) {
    config->leftActive = true;
    config->playerActive = false;

    config->mcOnly = false;
    config->clickInventory = false;
    config->breakBlocks = true;
    config->soundClicks = true;
}

// High-precision sleep function for accurate timing
void precisionSleep(double milliseconds) {
    if (milliseconds <= 0) return;
    
    LARGE_INTEGER frequency, start, current;
    if (!QueryPerformanceFrequency(&frequency)) {
        // Fallback to regular Sleep if high-precision timer unavailable
        Sleep((DWORD)milliseconds);
        return;
    }
    
    QueryPerformanceCounter(&start);
    double targetTicks = milliseconds * (double)frequency.QuadPart / 1000.0;
    
    // For delays > 2ms, use Sleep() for most of the time to avoid excessive CPU usage
    if (milliseconds > 2.0) {
        Sleep((DWORD)(milliseconds - 1.0)); // Sleep for most of the duration
        milliseconds = 1.0; // Spin-wait for the remaining time
        targetTicks = milliseconds * (double)frequency.QuadPart / 1000.0;
        QueryPerformanceCounter(&start); // Reset start time
    }
    
    // Spin-wait for precise timing (for the last 1-2ms or for very short delays)
    do {
        QueryPerformanceCounter(&current);
    } while ((current.QuadPart - start.QuadPart) < targetTicks);
}

// Clear screen using Windows console API (no external process = more stealth)
void clearScreen(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(
        hConsole,
        (TCHAR)' ',
        cellCount,
        homeCoords,
        &count
    )) return;

    if (!FillConsoleOutputAttribute(
        hConsole,
        csbi.wAttributes,
        cellCount,
        homeCoords,
        &count
    )) return;

    SetConsoleCursorPosition(hConsole, homeCoords);
}

bool cursorVisible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    if (!GetCursorInfo(&ci)) return false;
    return (ci.flags & CURSOR_SHOWING) != 0;
}

int bedrockCursorVisible() {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci)) {
        return false;
    }
    
    int systemVisible = (ci.flags & CURSOR_SHOWING) != 0;
    int handleValid = ci.hCursor != NULL;
    
    return systemVisible && handleValid;
}

void sendLeftClickDown(bool down) {
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

//
//SoundClicks
//

bool openWavFileDialog(WavCollection* collection) {
    OPENFILENAMEA ofn;
    char szFile[DIALOG_MULTISELECT_BUFFER] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = DIALOG_MULTISELECT_BUFFER;
    ofn.lpstrFilter = "WAV Files\0*.wav\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Select WAV Sound Files";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetOpenFileNameA(&ofn)) {
        collection->files = malloc(WAV_INIT_CAPACITY * sizeof(WavFile));
        collection->capacity = WAV_INIT_CAPACITY;
        collection->count = 0;

        char* directory = szFile;
        char* fileName = szFile + ofn.nFileOffset;
        
        if (szFile[ofn.nFileOffset - 1] == '\0') {
            while (*fileName) {
                char fullPath[MAX_PATH];
                sprintf_s(fullPath, sizeof(fullPath), "%s\\%s", directory, fileName);
                
                HANDLE hFile = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
                        char* buffer = malloc(fileSize);
                        if (buffer) {
                            DWORD bytesRead;
                            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                                if (collection->count >= collection->capacity) {
                                    collection->capacity *= 2;
                                    collection->files = realloc(collection->files, collection->capacity * sizeof(WavFile));
                                }
                                collection->files[collection->count].data = buffer;
                                collection->files[collection->count].size = fileSize;
                                collection->count++;
                                printf("Loaded: %s\n", fileName);
                            } else {
                                free(buffer);
                            }
                        }
                    }
                    CloseHandle(hFile);
                }
                fileName += strlen(fileName) + 1;
            }
        } else {
            HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fileSize = GetFileSize(hFile, NULL);
                if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
                    char* buffer = malloc(fileSize);
                    if (buffer) {
                        DWORD bytesRead;
                        if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                            collection->files[0].data = buffer;
                            collection->files[0].size = fileSize;
                            collection->count = 1;
                            printf("Loaded: %s\n", szFile);
                        } else {
                            free(buffer);
                        }
                    }
                }
                CloseHandle(hFile);
            }
        }
        
        return collection->count > 0;
    }
    return false;
}

void freeWavCollection(WavCollection* collection) {
    if (collection->files) {
        for (int i = 0; i < collection->count; i++) {
            if (collection->files[i].data) {
                free(collection->files[i].data);
            }
        }
        free(collection->files);
        collection->files = NULL;
    }
    collection->count = 0;
    collection->capacity = 0;
}

char* getRandomWavData(WavCollection* collection, DWORD* size) {
    if (collection->count == 0) {
        *size = 0;
        return NULL;
    }
    
    int index = rand() % collection->count;
    *size = collection->files[index].size;
    return collection->files[index].data;
}

// Open a file dialog to select a config file
char* openConfigFileDialog(void) {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Config Files\0*.txt;*.cfg;*.config\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Select Config File";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    
    if (GetOpenFileNameA(&ofn)) {
        return _strdup(szFile);
    }
    
    return NULL;
}