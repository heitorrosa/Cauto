#include "../resources/include.c"
#include "recorder.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

clickRecorder recorder;

// Helper functions
static void generate_random_string(char* output, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL) + GetTickCount());
    for (int i = 0; i < length; i++) {
        output[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    output[length] = '\0';
}

static void updateDisplay(int unified, int doubles, int total, LARGE_INTEGER firstClick, 
                         LARGE_INTEGER now, LARGE_INTEGER freq, bool showDebug) {
    clearScreen();
    printf("Click Recorder\nPress '%c' to stop and save\n\n", recorder.bindKey);
    printf("Recorded Clicks: %d (%d Double Clicks)\n", unified, doubles);
    
    if (firstClick.QuadPart != 0 && total > 1) {
        double timeSeconds = (double)(now.QuadPart - firstClick.QuadPart) / freq.QuadPart;
        double currentCPS = total / timeSeconds;
        printf("Current CPS: %.2f (Time: %.3fs, Total: %d, Recorded: %d)\n", 
               currentCPS, timeSeconds, total, unified);
        
        if (showDebug) {
            printf("DEBUG: First=%lld, Now=%lld, Freq=%lld\n", 
                   firstClick.QuadPart, now.QuadPart, freq.QuadPart);
        }
    }
}

static void processClickRelease(LARGE_INTEGER now, LARGE_INTEGER start, LARGE_INTEGER lastUp, 
                               LARGE_INTEGER freq, UnifiedClick* clicks, int* unified, 
                               int* doubles, int* total, int* nextId) {
    if (start.QuadPart == 0) return;
    
    double duration = (double)(now.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    (*total)++;
    
    if (lastUp.QuadPart != 0) {
        double gap = (double)(start.QuadPart - lastUp.QuadPart) * 1000.0 / freq.QuadPart;
        
        // Double click detection
        if (gap < 50.0) (*doubles)++;
        
        // Update previous click's delay or remove if gap too large
        if (duration <= 120.0 && gap <= 200.0 && *unified < MAX_CLICKS) {
            if (*unified > 0) clicks[*unified - 1].delay = gap;
        } else if (*unified > 0 && gap > 200.0) {
            (*unified)--;
        }
    }
    
    // Add current click if valid
    if (duration <= 120.0 && *unified < MAX_CLICKS) {
        clicks[*unified] = (UnifiedClick){*nextId, duration, 0.0, false};
        (*unified)++;
    }
    (*nextId)++;
}

static char* saveConfig(const char* configName, UnifiedClick* clicks, int unified, 
                       int total, int doubles, double finalCPS) {
    // Generate random filename (32 characters, no extension)
    char filename[256], randomName[33]; // 32 chars + null terminator
    generate_random_string(randomName, 32);
    strcpy(filename, randomName);
    
    printf("Generated filename: %s\nConfig name: %s\n", filename, configName);
    
    // Build config data
    char* buffer = malloc(100000000);
    if (!buffer) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }
    
    sprintf(buffer, "[CLICK_RECORDER_DATA]\nconfig_name=%s\ntotal_clicks=%d\n"
                   "double_clicks=%d\nunified_clicks=%d\naverage_cps=%.2f\n\n[UNIFIED_CLICKS]\n",
            configName, total, doubles, unified, finalCPS);
    
    // Add click data
    for (int i = 0; i < unified; i++) {
        char temp[128];
        sprintf(temp, "%d:%.3f:%.3f%s", clicks[i].id, clicks[i].duration, 
                clicks[i].delay, (i < unified - 1) ? "," : "");
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");
    
    // Encrypt and save
    int len = strlen(buffer);
    char* encrypted = malloc(len + 1);
    char* hexData = malloc(len * 2 + 1);
    
    if (encrypted && hexData) {
        xor_encrypt_decrypt(buffer, encrypted, len);
        binary_to_hex(encrypted, hexData, len);
        
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", hexData);
            fclose(file);
            printf("Encrypted data saved to '%s'\n", filename);
        } else {
            printf("Error: Could not save file '%s'\n", filename);
        }
    }
    
    free(buffer);
    free(encrypted);
    return hexData;
}

// Main recording function - keeps all original features
char* recordClicks() {
    // Initialize variables
    LARGE_INTEGER freq, start = {0}, lastUp = {0}, firstClick = {0};
    static UnifiedClick clicks[MAX_CLICKS];
    int unified = 0, doubles = 0, total = 0, nextId = 1;
    BOOL wasPressed = FALSE;
    
    if (!QueryPerformanceFrequency(&freq)) {
        printf("Error: Performance counter not available.\n");
        return NULL;
    }
    
    // Initial display
    updateDisplay(unified, doubles, total, firstClick, firstClick, freq, false);
    
    printf("Press '%c' to start recording...\n", recorder.bindKey);
    
    // Wait for bind key to start recording
    while (!(GetAsyncKeyState(recorder.bindKey) & 0x8000)) {
        Sleep(10);
    }
    
    // Wait for key release to avoid immediate retrigger
    while (GetAsyncKeyState(recorder.bindKey) & 0x8000) {
        Sleep(10);
    }
    
    if (recorder.beepOnStart == 'Y' || recorder.beepOnStart == 'y') {
        Beep(1000, 500);
    }
    
    printf("Recording started! Press '%c' again to stop and save...\n", recorder.bindKey);
    
    // Main recording loop - toggle with bind key
    while (!(GetAsyncKeyState(recorder.bindKey) & 0x8000)) {
        BOOL isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        
        if (isPressed && !wasPressed) {
            // Button pressed
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            if (firstClick.QuadPart == 0) firstClick = now;
            start = now;
            updateDisplay(unified, doubles, total, firstClick, now, freq, false);
        }
        else if (!isPressed && wasPressed) {
            // Button released
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            processClickRelease(now, start, lastUp, freq, clicks, &unified, &doubles, &total, &nextId);
            updateDisplay(unified, doubles, total, firstClick, now, freq, true);
            lastUp = now;
        }
        
        wasPressed = isPressed;
        Sleep(1);
    }
    
    // Wait for key release
    while (GetAsyncKeyState(recorder.bindKey) & 0x8000) {
        Sleep(10);
    }
    
    if (recorder.beepOnStart == 'Y' || recorder.beepOnStart == 'y') {
        Beep(500, 300);  // Different tone for stop
    }
    
    // Final statistics
    printf("\nFinal Statistics\nTotal clicks: %d (%d Double clicks)\n", total, doubles);
    
    double finalCPS = 0.0;
    if (firstClick.QuadPart != 0 && total > 0) {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        double timeSeconds = (double)(end.QuadPart - firstClick.QuadPart) / freq.QuadPart;
        finalCPS = total / timeSeconds;
        printf("Average CPS: %.2f\n", finalCPS);
    }
    
    // Get config name
    char configName[256];
    printf("\nEnter a name for this click configuration: ");
    fflush(stdout);
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (fgets(configName, sizeof(configName), stdin)) {
        configName[strcspn(configName, "\n")] = 0;
        if (strlen(configName) == 0) strcpy(configName, "DefaultConfig");
    } else {
        strcpy(configName, "DefaultConfig");
    }
    
    return saveConfig(configName, clicks, unified, total, doubles, finalCPS);
}