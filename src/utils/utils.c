#include "../resources/include.c"

#include "utils.h"

void initGlobalConfig(globalConfig *config) {
    config->leftActive = false;
    config->playerActive = false;

    config->mcOnly = true;
    config->clickInventory = false;
    config->breakBlocks = true;
    config->soundClicks = true;
}

bool cursorVisible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;
    
    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void sendLeftClickDown(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    PostMessageA(GetForegroundWindow(), down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}

//SoundClicks

bool openWavFileDialog(WavCollection* collection) {
    OPENFILENAMEA ofn;
    char szFile[32768] = {0}; // Large buffer for multiple files

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "WAV Files\0*.wav\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Select WAV Sound Files";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetOpenFileNameA(&ofn)) {
        collection->files = malloc(10 * sizeof(WavFile)); // Initial capacity
        collection->capacity = 10;
        collection->count = 0;

        // Check if multiple files selected by looking at the buffer structure
        char* directory = szFile;
        char* fileName = szFile + ofn.nFileOffset;
        
        // If there's a null terminator before nFileOffset, multiple files were selected
        if (szFile[ofn.nFileOffset - 1] == '\0') {
            // Multiple files - first string is directory, then individual filenames
            while (*fileName) {
                char fullPath[MAX_PATH];
                sprintf_s(fullPath, sizeof(fullPath), "%s\\%s", directory, fileName);
                
                // Load WAV file into memory
                HANDLE hFile = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    char* buffer = malloc(fileSize);
                    
                    if (buffer) {
                        DWORD bytesRead;
                        if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                            // Resize array if needed
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
                    CloseHandle(hFile);
                }
                
                // Move to next filename
                fileName += strlen(fileName) + 1;
            }
        } else {
            // Single file - the entire szFile is the path
            HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fileSize = GetFileSize(hFile, NULL);
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