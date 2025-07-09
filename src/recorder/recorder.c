#include "../resources/include.c"

#include "recorder.h"
#include "../utils/utils.h"

// XOR encryption key
static const char XOR_KEY[] = "6sVKlW/RqcfH5ZbOWF/jjkb7jBbfTZJmgU8fNX/+tpH+b8BXfy08f0RjwMZLvLI/okeC/fZlWpZiNjunOSVfFnHp00vJRcejZ9OuBHJa0M2FO/kQqAU6WaYlPM6CJh5WCtMaVHMSCa64Z0cxQs1RWXuPoERikrMY8KhvANc7lood9IhUd+ZvJ4RfD4rgaBev";

// Function to encrypt/decrypt data using XOR cipher
void xor_encrypt_decrypt(const char* input, char* output, int length) {
    int key_len = strlen(XOR_KEY);
    for (int i = 0; i < length; i++) {
        output[i] = input[i] ^ XOR_KEY[i % key_len];
    }
}

// Function to convert binary data to hex string
void binary_to_hex(const char* binary, char* hex, int length) {
    for (int i = 0; i < length; i++) {
        sprintf(hex + (i * 2), "%02X", (unsigned char)binary[i]);
    }
    hex[length * 2] = '\0';
}

// Function to convert hex string to binary data
int hex_to_binary(const char* hex, char* binary) {
    int len = strlen(hex);
    for (int i = 0; i < len; i += 2) {
        char hex_byte[3] = {hex[i], hex[i + 1], '\0'};
        binary[i / 2] = (char)strtol(hex_byte, NULL, 16);
    }
    return len / 2;
}


// Arrays to store data
#define MAX_CLICKS 1000000
static UnifiedClick unifiedClicks[MAX_CLICKS];
static int unifiedClickCount = 0;
static int doubleClicks = 0;
static int totalClicks = 0;

int recordClicks() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER startTime = {0};
    LARGE_INTEGER lastUpTime = {0};
    LARGE_INTEGER firstClickTime = {0};
    BOOL wasPressed = FALSE;
    double currentClickDuration = 0.0;
    
    // Initialize performance counter
    if (!QueryPerformanceFrequency(&frequency)) {
        printf("Error: Performance counter not available.\n");
        return 1;
    }
    
    printf("Monitoring mouse clicks - Press 'INSERT' to quit...\n");
    
    while (true) {
            if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
                break;
            }
        
        BOOL isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        
        if (isPressed && !wasPressed) {
            // Button just pressed
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            
            // Record first click time for CPS calculation
            if (firstClickTime.QuadPart == 0) {
                firstClickTime = now;
            }
            
            startTime = now;
        }
        else if (!isPressed && wasPressed) {
            // Button just released
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            
            if (startTime.QuadPart != 0) {
                double duration = (double)(now.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
                
                currentClickDuration = duration;
                totalClicks++;
                
                // If this is not the first click, calculate delay from previous click
                if (lastUpTime.QuadPart != 0) {
                    double gap = (double)(startTime.QuadPart - lastUpTime.QuadPart) * 1000.0 / frequency.QuadPart;
                    
                    // Check for double click
                    if (gap < 10.0) {
                        doubleClicks++;
                        printf("Double click detected!\n");
                    }
                    
                    // Create unified click if both duration and delay meet requirements
                    if (currentClickDuration <= 120.0 && gap <= 200.0 && unifiedClickCount < MAX_CLICKS) {
                        // Update the previous click's delay (it was waiting for this information)
                        if (unifiedClickCount > 0) {
                            unifiedClicks[unifiedClickCount - 1].delay = gap;
                        }
                    } else {
                        // Remove the previous click if delay doesn't meet requirements
                        if (unifiedClickCount > 0 && gap > 200.0) {
                            unifiedClickCount--;
                            printf("Previous click removed due to excessive delay\n");
                        }
                    }
                }
                
                // Add current click if duration meets requirements
                if (currentClickDuration <= 120.0 && unifiedClickCount < MAX_CLICKS) {
                    unifiedClicks[unifiedClickCount].duration = currentClickDuration;
                    unifiedClicks[unifiedClickCount].delay = 0.0; // Will be set when next click happens
                    unifiedClickCount++;
                } else if (currentClickDuration > 120.0) {
                    printf("Click discarded - duration too long\n");
                }
            }
            
            lastUpTime = now;
        }
        
        wasPressed = isPressed;
        Sleep(1); // Small delay to prevent excessive CPU usage
    }
    
    // Calculate and display statistics
    printf("\n--- Final Statistics ---\n");
    printf("Total clicks: %d (%d Double clicks)\n", totalClicks, doubleClicks);
    
    // Calculate CPS (Clicks Per Second)
    if (firstClickTime.QuadPart != 0 && totalClicks > 0) {
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);
        double totalTimeSeconds = (double)(endTime.QuadPart - firstClickTime.QuadPart) / frequency.QuadPart;
        double cps = totalClicks / totalTimeSeconds;
        printf("Average CPS: %.2f\n", cps);
    }
    
    // Ask user for filename
    char filename[256];
    printf("\nEnter the name for the configuration file (without extension): ");
    fflush(stdout); // Force output before input
    
    // Clear any leftover input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (fgets(filename, sizeof(filename), stdin) != NULL) {
        // Remove newline character
        filename[strcspn(filename, "\n")] = 0;
        
        // Add .cauto extension if not present
        if (!strstr(filename, ".cauto")) {
            strcat(filename, ".cauto");
        }
    } else {
        strcpy(filename, "config.cauto");
    }
    
    printf("Saving to file: %s\n", filename);
    
    // Create temporary buffer for unencoded data
    char* tempBuffer = malloc(100000000); // 100MB buffer
    if (!tempBuffer) {
        printf("\nError: Could not allocate memory for encoding\n");
        return 1;
    }
    
    // Build the data string
    sprintf(tempBuffer, "[CLICK_RECORDER_DATA]\ntotal_clicks=%d\ndouble_clicks=%d\nunified_clicks=%d\naverage_cps=%.2f\n\n[UNIFIED_CLICKS]\n",
            totalClicks, doubleClicks, unifiedClickCount,
            (firstClickTime.QuadPart != 0 && totalClicks > 0) ? 
            totalClicks / ((double)(GetTickCount64() - firstClickTime.QuadPart) / frequency.QuadPart) : 0.0);
    
    // Add unified clicks (duration:delay pairs)
    for (int i = 0; i < unifiedClickCount; i++) {
        char temp[64];
        sprintf(temp, "%.3f:%.3f", unifiedClicks[i].duration, unifiedClicks[i].delay);
        strcat(tempBuffer, temp);
        if (i < unifiedClickCount - 1) strcat(tempBuffer, ",");
    }
    strcat(tempBuffer, "\n");
    
    // Encrypt using XOR cipher
    int tempLen = strlen(tempBuffer);
    char* encryptedBuffer = malloc(tempLen + 1);
    if (!encryptedBuffer) {
        printf("\nError: Could not allocate memory for encryption\n");
        free(tempBuffer);
        return 1;
    }
    
    xor_encrypt_decrypt(tempBuffer, encryptedBuffer, tempLen);
    encryptedBuffer[tempLen] = '\0';
    
    // Convert to hex string for file storage
    char* hexBuffer = malloc(tempLen * 2 + 1);
    if (!hexBuffer) {
        printf("\nError: Could not allocate memory for hex conversion\n");
        free(tempBuffer);
        free(encryptedBuffer);
        return 1;
    }
    
    binary_to_hex(encryptedBuffer, hexBuffer, tempLen);
    
    // Save encrypted data to file
    FILE* configFile = fopen(filename, "w");
    if (configFile != NULL) {
        fprintf(configFile, "%s", hexBuffer);
        
        fclose(configFile);
        printf("\nEncrypted data saved to '%s'\n", filename);
    } else {
        printf("\nError: Could not save data to file '%s'\n", filename);
    }
    
    // Cleanup
    free(tempBuffer);
    free(encryptedBuffer);
    free(hexBuffer);
    
    printf("\nProgram completed. Press any key to exit...");
    _getch();
    return 0;
}