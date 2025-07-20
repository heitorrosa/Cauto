#include "../resources/include.c"
#include "recorder.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

// Helper function to convert character to virtual key code
static int charToVirtualKey(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    if (c >= 'A' && c <= 'Z') return c;
    if (c >= '0' && c <= '9') return c;
    
    switch (c) {
        case ' ': return VK_SPACE;
        case '\t': return VK_TAB;
        case '\n': return VK_RETURN;
        default: return toupper(c);
    }
}

// Generate random filename
static void generateRandomName(char* output, int length) {
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

// Save configuration with encryption and pause filtering
static char* saveConfig(const char* configName, RecordedClick* clicks, int clickCount) {
    // Filter out clicks with long pauses and calculate statistics
    int filteredCount = 0;
    double totalTime = 0.0;
    int longPausesFiltered = 0;
    
    // Count valid clicks and calculate total time
    for (int i = 0; i < clickCount; i++) {
        if (i == 0) {
            // First click is always included
            filteredCount++;
            totalTime += clicks[i].duration;
        } else {
            if (clicks[i].delay <= PAUSE_THRESHOLD_MS) {
                // Normal delay, include this click
                clicks[i].isFiltered = false;
                filteredCount++;
                totalTime += clicks[i].delay + clicks[i].duration;
            } else {
                // Long pause detected, mark as filtered
                clicks[i].isFiltered = true;
                longPausesFiltered++;
                // Still count the click itself, but use average delay instead
                filteredCount++;
                // Calculate average delay from previous valid clicks
                double avgDelay = 0.0;
                int validDelays = 0;
                for (int j = 1; j < i; j++) {
                    if (!clicks[j].isFiltered && clicks[j].delay <= PAUSE_THRESHOLD_MS) {
                        avgDelay += clicks[j].delay;
                        validDelays++;
                    }
                }
                if (validDelays > 0) {
                    avgDelay /= validDelays;
                    clicks[i].delay = avgDelay; // Replace long pause with average delay
                } else {
                    clicks[i].delay = 80.0; // Fallback to 80ms if no valid delays yet
                }
                totalTime += clicks[i].delay + clicks[i].duration;
            }
        }
    }
    
    totalTime /= 1000.0; // Convert to seconds
    double averageCPS = (filteredCount > 1) ? (filteredCount - 1) / totalTime : 0.0;
    
    printf("\n=== Pause Filtering Results ===\n");
    printf("Long pauses filtered: %d (over %.0fms)\n", longPausesFiltered, PAUSE_THRESHOLD_MS);
    printf("Final click count: %d\n", filteredCount);
    printf("Adjusted average CPS: %.2f\n", averageCPS);
    
    // Generate random filename
    char filename[256], randomName[33];
    generateRandomName(randomName, 32);
    sprintf(filename, "%s.txt", randomName);
    
    printf("\nSaving configuration...\n");
    printf("Filename: %s\n", filename);
    printf("Config name: %s\n", configName);
    
    // Build config data
    char* buffer = malloc(100000); // 100KB buffer should be enough
    if (!buffer) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }
    
    // Create config header
    sprintf(buffer, "Config: %s\nClicks: %d\nAverage CPS: %.2f\n\n", 
            configName, filteredCount, averageCPS);
    
    // Add click data (only non-filtered or adjusted clicks)
    int outputIndex = 1;
    for (int i = 0; i < clickCount; i++) {
        char clickLine[100];
        sprintf(clickLine, "%d, %.2f, %.2f\n", 
                outputIndex, clicks[i].duration, clicks[i].delay);
        strcat(buffer, clickLine);
        outputIndex++;
    }
    
    // Encrypt and save
    int len = strlen(buffer);
    char* encrypted = malloc(len + 1);
    char* hexData = malloc(len * 2 + 1);
    
    if (encrypted && hexData) {
        // Encrypt with XOR
        XOREncryptDecrypt(buffer, encrypted, len);
        
        // Convert to hex
        BinaryToHex(encrypted, hexData, len);
        
        // Save to file
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", hexData);
            fclose(file);
            printf("Configuration saved successfully!\n");
            
            // Return hex data for immediate use
            char* result = strdup(hexData);
            free(buffer);
            free(encrypted);
            free(hexData);
            return result;
        } else {
            printf("Error: Could not save file\n");
        }
    }
    
    free(buffer);
    free(encrypted);
    free(hexData);
    return NULL;
}

char* recordClicks(RecorderConfig* config) {
    // High-precision timing setup
    LARGE_INTEGER frequency, startTime, endTime;
    if (!QueryPerformanceFrequency(&frequency)) {
        printf("Error: High-precision timer not available\n");
        return NULL;
    }
    
    // Allocate memory for recorded clicks
    RecordedClick* clicks = malloc(MAX_CLICKS * sizeof(RecordedClick));
    if (!clicks) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }
    
    int clickCount = 0;
    bool isPressed = false;
    int bindKeyVK = charToVirtualKey(config->bindKey);
    
    // Window detection for Minecraft-only mode
    HWND currentWindow, minecraftRecent, minecraftOld, minecraftBedrock;
    
    clearScreen();
    printf("=== Smart Click Recorder ===\n");
    printf("Automatic pause filtering: Delays over %.0fms will be filtered\n", PAUSE_THRESHOLD_MS);
    printf("Press '%c' to start recording...\n", config->bindKey);
    
    // Wait for bind key to start
    while (!(GetAsyncKeyState(bindKeyVK) & 0x8000)) {
        Sleep(10);
    }
    while (GetAsyncKeyState(bindKeyVK) & 0x8000) {
        Sleep(10);
    }
    
    if (config->beepOnStart) {
        Beep(800, 200);
    }
    
    clearScreen();
    printf("=== Recording Started ===\n");
    printf("Press '%c' to stop recording\n", config->bindKey);
    printf("Left-click to record clicks...\n");
    printf("Long pauses (>%.0fms) will be automatically filtered\n\n", PAUSE_THRESHOLD_MS);

    LARGE_INTEGER lastClickEnd = {0};
    
    // Main recording loop
    while (!(GetAsyncKeyState(bindKeyVK) & 0x8000)) {
        // Check Minecraft window if mcOnly is enabled
        if (config->mcOnly) {
            currentWindow = GetForegroundWindow();
            minecraftRecent = FindWindowA("GLFW30", NULL);
            minecraftOld = FindWindowA("LWJGL", NULL);
            minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL);
            
            if (currentWindow != minecraftRecent && 
                currentWindow != minecraftOld && 
                currentWindow != minecraftBedrock) {
                Sleep(10);
                continue;
            }
        }
        
        bool currentlyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        
        if (currentlyPressed && !isPressed) {
            // Click started
            QueryPerformanceCounter(&startTime);
            isPressed = true;
            
        } else if (!currentlyPressed && isPressed) {
            // Click ended
            QueryPerformanceCounter(&endTime);
            isPressed = false;
            
            if (clickCount < MAX_CLICKS) {
                // Calculate duration
                double duration = getTimeDifferenceMs(startTime, endTime, frequency);
                
                // Calculate delay from previous click
                double delay = 0.0;
                bool isLongPause = false;
                if (clickCount > 0 && lastClickEnd.QuadPart > 0) {
                    delay = getTimeDifferenceMs(lastClickEnd, startTime, frequency);
                    isLongPause = (delay > PAUSE_THRESHOLD_MS);
                }
                
                // Store click data
                clicks[clickCount].duration = duration;
                clicks[clickCount].delay = delay;
                clicks[clickCount].isFiltered = false; // Will be set during save
                clickCount++;
                
                // Update display with pause indication
                if (isLongPause) {
                    printf("Click %d: Duration=%.1fms, Delay=%.1fms [LONG PAUSE - Will be filtered]\n", 
                           clickCount, duration, delay);
                } else {
                    printf("Click %d: Duration=%.1fms, Delay=%.1fms\n", 
                           clickCount, duration, delay);
                }
                
                lastClickEnd = endTime;
            }
        }
        
        Sleep(1); // Small sleep to prevent excessive CPU usage
    }
    
    // Wait for bind key release
    while (GetAsyncKeyState(bindKeyVK) & 0x8000) {
        Sleep(10);
    }
    
    if (config->beepOnStart) {
        Beep(600, 200);
    }
    
    if (clickCount == 0) {
        printf("No clicks recorded!\n");
        free(clicks);
        return NULL;
    }
    
    printf("\n=== Recording Complete ===\n");
    printf("Total Clicks Recorded: %d\n", clickCount);
    
    // Get config name
    char configName[256];
    printf("\nEnter config name: ");
    fflush(stdin);
    if (fgets(configName, sizeof(configName), stdin)) {
        // Remove newline
        configName[strcspn(configName, "\n")] = '\0';
        if (strlen(configName) == 0) {
            strcpy(configName, "Recorded Config");
        }
    } else {
        strcpy(configName, "Recorded Config");
    }
    
    // Save and return encrypted config with pause filtering
    char* result = saveConfig(configName, clicks, clickCount);
    
    free(clicks);
    return result;
}
