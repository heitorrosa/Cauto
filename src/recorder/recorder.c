#include "../resources/include.c"
#include "recorder.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

// High-precision timing structure
typedef struct {
    LARGE_INTEGER timestamp;    // High-precision timestamp
    double duration;           // Click duration in milliseconds
    double interval;          // Time to next click in milliseconds
} PrecisionClick;

// Helper function to convert character to virtual key code
static int charToVirtualKey(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    if (c >= 'A' && c <= 'Z') return c;
    if (c >= '0' && c <= '9') return c;
    
    switch (c) {
        case ' ': return VK_SPACE;
        case '\t': return VK_TAB;
        case '\r': case '\n': return VK_RETURN;
        default: return VK_INSERT; // Default fallback
    }
}

// Generate random filename
static void generate_random_string(char* output, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL) + GetTickCount());
    for (int i = 0; i < length; i++) {
        output[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    output[length] = '\0';
}

// High-precision time difference in milliseconds
static double getTimeDifferenceMs(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency) {
    return ((double)(end.QuadPart - start.QuadPart) * 1000.0) / (double)frequency.QuadPart;
}

// Update display without real-time CPS calculation
static void updateDisplay(PrecisionClick* clicks, int clickCount, int doubleClicks, 
                         LARGE_INTEGER frequency, bool showStats, clickRecorder* rec) {
    clearScreen();
    printf("=== Click Recorder ===\n");
    printf("Press '%c' to stop recording and save\n\n", rec->bindKey);
    
    printf("Recorded Clicks: %d\n", clickCount);
    printf("Double Clicks: %d\n", doubleClicks);
    
    printf("\nStatus: Recording... (Hold left mouse button and click)\n");
}

// Save configuration with proper format
static char* saveConfig(const char* configName, UnifiedClick* clicks, int unified, 
                       int total, int doubles, double finalCPS) {
    // Generate random filename
    char filename[256], randomName[33];
    generate_random_string(randomName, 32);
    strcpy(filename, randomName);
    
    printf("\nSaving configuration...\n");
    printf("Filename: %s\n", filename);
    printf("Config name: %s\n", configName);
    
    // Build config data
    char* buffer = malloc(10000000); // 10MB buffer
    if (!buffer) {
        printf("Error: Could not allocate memory for config data\n");
        return NULL;
    }
    
    // Header
    sprintf(buffer, 
        "[CLICK_RECORDER_DATA]\n"
        "config_name=%s\n"
        "total_clicks=%d\n"
        "double_clicks=%d\n"
        "unified_clicks=%d\n"
        "average_cps=%.2f\n"
        "[UNIFIED_CLICKS]\n",
        configName, total, doubles, unified, finalCPS);
    
    // Add click data
    for (int i = 0; i < unified; i++) {
        char clickStr[64];
        sprintf(clickStr, "%d:%.3f:%.3f", 
                clicks[i].id, 
                clicks[i].duration, 
                clicks[i].delay);
        
        strcat(buffer, clickStr);
        if (i < unified - 1) {
            strcat(buffer, ",");
        }
    }
    strcat(buffer, "\n");
    
    // Encrypt and save
    int len = strlen(buffer);
    char* encrypted = malloc(len + 1);
    char* hexData = malloc(len * 2 + 1);
    
    if (encrypted && hexData) {
        XOREncryptDecrypt(buffer, encrypted, len);
        BinaryToHex(encrypted, hexData, len);
        
        // Save to file
        FILE* file;
        if (fopen_s(&file, filename, "w") == 0) {
            fprintf(file, "%s", hexData);
            fclose(file);
            printf("Config saved successfully to: %s\n", filename);
        } else {
            printf("Error: Could not save config to file\n");
        }
        
        free(encrypted);
        free(buffer);
        return hexData;
    }
    
    free(buffer);
    return NULL;
}

char* recordClicks(clickRecorder* rec) {
    // High-precision timing setup
    LARGE_INTEGER frequency, startTime, endTime;
    if (!QueryPerformanceFrequency(&frequency)) {
        printf("Error: High-precision timer not available\n");
        return NULL;
    }
    
    // Allocate memory for precision clicks
    PrecisionClick* precisionClicks = malloc(MAX_CLICKS * sizeof(PrecisionClick));
    if (!precisionClicks) {
        printf("Error: Could not allocate memory for clicks\n");
        return NULL;
    }
    
    int clickCount = 0;
    int doubleClicks = 0;
    bool isPressed = false;
    bool wasPressed = false;
    int bindKeyVK = charToVirtualKey(rec->bindKey);
    
    // Window detection for Minecraft-only mode
    HWND currentWindow, minecraftRecent, minecraftOld, minecraftBedrock;
    
    updateDisplay(precisionClicks, clickCount, doubleClicks, frequency, false, rec);
    printf("Press '%c' to start recording...\n", rec->bindKey);
    
    // Wait for bind key to start
    while (!(GetAsyncKeyState(bindKeyVK) & 0x8000)) {
        precisionSleep(1.0);
    }
    while (GetAsyncKeyState(bindKeyVK) & 0x8000) {
        precisionSleep(1.0);
    }
    
    if (rec->beepOnStart == 'Y' || rec->beepOnStart == 'y') {
        Beep(800, 200);
    }
    
    printf("Recording started! Left-click to record clicks...\n");
    
    LARGE_INTEGER lastClickStart = {0};
    
    // Main recording loop
    while (!(GetAsyncKeyState(bindKeyVK) & 0x8000)) {
        // Check for window focus if Minecraft-only mode
        if (rec->mcOnly == 'Y' || rec->mcOnly == 'y') {
            currentWindow = GetForegroundWindow();
            minecraftRecent = FindWindowA("GLFW30", NULL);
            minecraftOld = FindWindowA("LWJGL", NULL); 
            minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL);
            
            if (currentWindow != minecraftRecent && 
                currentWindow != minecraftOld && 
                currentWindow != minecraftBedrock) {
                precisionSleep(1.0);
                continue;
            }
        }
        
        wasPressed = isPressed;
        isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        
        // Mouse button pressed (click start)
        if (isPressed && !wasPressed) {
            QueryPerformanceCounter(&startTime);
            lastClickStart = startTime;
        }
        // Mouse button released (click end)
        else if (!isPressed && wasPressed && clickCount < MAX_CLICKS) {
            QueryPerformanceCounter(&endTime);
            
            // Calculate click duration
            double duration = getTimeDifferenceMs(lastClickStart, endTime, frequency);
            
            // Only record clicks with reasonable duration (5-200ms)
            if (duration >= 5.0 && duration <= 200.0) {
                precisionClicks[clickCount].timestamp = lastClickStart;
                precisionClicks[clickCount].duration = duration;
                
                // Calculate and set interval from previous click (if exists)
                if (clickCount > 0) {
                    double interval = getTimeDifferenceMs(precisionClicks[clickCount-1].timestamp, 
                                                         lastClickStart, frequency);
                    
                    // For very long pauses (>500ms), cap the interval to maintain reasonable playback
                    if (interval > 500.0) {
                        printf("Very long pause detected (%.1fms), capping interval for natural playback\n", interval);
                        interval = 150.0; // Cap at reasonable interval
                    }
                    
                    precisionClicks[clickCount-1].interval = interval;
                    
                    // Check for double click (interval < 50ms for actual double-clicks)
                    if (interval < 50.0) {
                        doubleClicks++;
                    }
                }
                
                clickCount++;
                
                // Update display every click for better feedback
                updateDisplay(precisionClicks, clickCount, doubleClicks, frequency, true, rec);
            }
        }
        
        precisionSleep(1.0); // Small delay to prevent excessive CPU usage
    }
    
    // Wait for bind key release
    while (GetAsyncKeyState(bindKeyVK) & 0x8000) {
        precisionSleep(1.0);
    }
    
    if (rec->beepOnStart == 'Y' || rec->beepOnStart == 'y') {
        Beep(600, 200);
    }
    
    if (clickCount == 0) {
        printf("\nNo clicks recorded!\n");
        free(precisionClicks);
        return NULL;
    }
    
    // Calculate simple final statistics
    double totalTime = getTimeDifferenceMs(precisionClicks[0].timestamp, 
                                         precisionClicks[clickCount-1].timestamp, frequency) / 1000.0;
    double simpleCPS = (double)(clickCount - 1) / totalTime;
    
    printf("\n=== Recording Complete ===\n");
    printf("Total clicks: %d\n", clickCount);
    printf("Double clicks: %d\n", doubleClicks);
    printf("Total recording time: %.2f seconds\n", totalTime);
    printf("Average CPS: %.2f\n", simpleCPS);
    
    // Get config name
    char configName[256];
    printf("\nEnter config name: ");
    fflush(stdin);
    if (fgets(configName, sizeof(configName), stdin)) {
        configName[strcspn(configName, "\n")] = '\0';
    }
    if (strlen(configName) == 0) {
        strcpy(configName, "Recorded_Config");
    }
    
    // Convert precision clicks to unified clicks format
    UnifiedClick* unifiedClicks = malloc(clickCount * sizeof(UnifiedClick));
    if (!unifiedClicks) {
        printf("Error: Could not allocate memory for unified clicks\n");
        free(precisionClicks);
        return NULL;
    }
    
    for (int i = 0; i < clickCount; i++) {
        unifiedClicks[i].id = i + 1;
        unifiedClicks[i].duration = precisionClicks[i].duration;
        unifiedClicks[i].delay = (i < clickCount - 1) ? precisionClicks[i].interval : 0.0;
        unifiedClicks[i].hasDelay = (i < clickCount - 1);
    }
    
    // Save configuration
    char* result = saveConfig(configName, unifiedClicks, clickCount, clickCount, doubleClicks, simpleCPS);
    
    free(precisionClicks);
    free(unifiedClicks);
    return result;
}
