/*
assign an id for each itteration
get the total time from mouseDown (clickDuration) and mouseUp (delayBetweenClicks)
make a condition check based on the threasholds defined in clickRecorder_config (clickRecorder.delayBetweenClicks_threashold and clickRecorder.clickDuration_threashold)
if any of them fails, discard the entire click and move on to the next one
if it succeed, increment the clickData vector with the respective values in milliseconds from clickDuration and delayBetweenClicks (recorderInstance.clickData[id].clickDuration = clickDuration)
and recorderInstance.clickCout++
if the user triggers the clickRecorder.keybind when recorderInstance.recording, it should make the user choose a name for the config and save it as a txt file in xor encrypted for later load.

also condition check the related clickRecorder.mcOnly and clickRecorder.beepOnStart and do its actions based on their values.

the clickRecorderHander(void) function should be used to execute the whole recorder task, so, I can call it in just one function, like in main.c, where i call leftClickerHandler() for the leftClicker, instead of bloating the code out. For the case of the clickRecorderHandler(void) it should also have something like a while loop to prevent me from going to the main.c loop, since in calling it from the menu.c file
*/

#include "common.h"

clickRecorder_handler recorderInstance;

// Helper function to convert char to virtual key
int charToVKey(char c) {
    if(c >= 'A' && c <= 'Z') return c;
    if(c >= 'a' && c <= 'z') return c - 32;
    if(c >= '0' && c <= '9') return c;
    
    switch(c) {
        case ' ': return VK_SPACE;
        case '\t': return VK_TAB;
        case '\r': return VK_RETURN;
        case '\x1b': return VK_ESCAPE;
        default: return c;
    }
}

// Helper function to XOR encrypt/decrypt
void xorEncrypt(char *data, int length, char key) {
    for(int i = 0; i < length; i++) {
        data[i] ^= key;
    }
}

// Helper function to save config to encrypted file
void saveRecordedConfig() {
    char filename[256];
    
    printf("\n\nEnter config name: ");
    scanf_s("%255s", recorderInstance.configName, sizeof(recorderInstance.configName));
    
    strcpy_s(filename, sizeof(filename), recorderInstance.configName);
    strcat_s(filename, sizeof(filename), ".txt");

    FILE *file;
    if(fopen_s(&file, filename, "w") == 0) {
        // Prepare data to encrypt
        char buffer[4096] = {0};
        int offset = 0;
        
        // Format: configName\nclickCount,averageCPS\nclickDuration,delayBetweenClicks\n...
        offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%s\n", recorderInstance.configName);
        offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%d,%.2f\n", recorderInstance.clickCout, recorderInstance.averageCPS);
        
        for(int i = 0; i < recorderInstance.clickCout; i++) {
            offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%d,%d\n", 
                recorderInstance.clickData[i].clickDuration,
                recorderInstance.clickData[i].delayBetweenClicks);
        }
        
        // Encrypt
        xorEncrypt(buffer, offset, 0x47);
        
        // Write as hex string for readability
        for(int i = 0; i < offset; i++) {
            fprintf(file, "%02X", (unsigned char)buffer[i]);
        }
        fclose(file);

        printf("\nConfig saved as: %s\n", filename);
    } else {
        printf("\nError opening file for writing: %s\n", filename);
    }
}

// Main click recorder handler
void clickRecorderHandler() {
    int vKey = charToVKey(clickRecorder.keybind);
    
    printf("\nPress '%c' to start recording...\n", clickRecorder.keybind);
    
    // Wait for keybind press to start recording
    while(1) {
        if(GetAsyncKeyState(vKey) & 0x8000) {
            robustSleep(0.2); // Debounce
            break;
        }
        robustSleep(0.001);
    }

    recorderInstance.recording = true;
    recorderInstance.clickCout = 0;
    recorderInstance.clickData = malloc(sizeof(*recorderInstance.clickData) * 1000);

    if(clickRecorder.beepOnStart) {
        Beep(1000, 100);
    }

    printf("Recording clicks... Press '%c' to stop and save.\n", clickRecorder.keybind);

    DWORD lastMouseUpTime = 0;
    DWORD currentClickDownTime = 0;
    bool keybindPressed = false;

    while(recorderInstance.recording) {
        // Check for stop keybind
        if(GetAsyncKeyState(vKey) & 0x8000) {
            if(!keybindPressed) {
                keybindPressed = true;
            }
        } else {
            if(keybindPressed) {
                recorderInstance.recording = false;
                break;
            }
        }

        // Check minecraft only setting
        if(clickRecorder.mcOnly) {
            if(GetForegroundWindow() != FindWindowA(MINECRAFT_RECENT, NULL) &&
               GetForegroundWindow() != FindWindowA(MINECRAFT_OLD, NULL) &&
               GetForegroundWindow() != FindWindowA(MINECRAFT_BEDROCK, NULL)) robustSleep(0.001);
        }

        // Detect mouse button press
        if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if(currentClickDownTime == 0) {
                currentClickDownTime = GetTickCount();
            }
        } else {
            // Mouse button released
            if(currentClickDownTime != 0) {
                DWORD clickDownTime = GetTickCount();
                int clickDuration = clickDownTime - currentClickDownTime;
                int delayBetweenClicks = (lastMouseUpTime == 0) ? 0 : (currentClickDownTime - lastMouseUpTime);

                // Validate against thresholds
                if(clickDuration <= clickRecorder.clickDuration_threshold &&
                   delayBetweenClicks <= clickRecorder.delayBetweenClicks_threashold) {

                    recorderInstance.clickData[recorderInstance.clickCout].clickDuration = clickDuration;
                    recorderInstance.clickData[recorderInstance.clickCout].delayBetweenClicks = delayBetweenClicks;
                    recorderInstance.clickCout++;
                } else {
                    printf("\n[DEBUG] Click ignored - Duration: %dms (threshold: %dms), Delay: %dms (threshold: %dms)\n",
                        clickDuration, clickRecorder.clickDuration_threshold,
                        delayBetweenClicks, clickRecorder.delayBetweenClicks_threashold);
                }

                lastMouseUpTime = clickDownTime;
                currentClickDownTime = 0;
            }
        }

        robustSleep(0.001);
    }

    // Calculate average CPS
    if(recorderInstance.clickCout > 0) {
        int totalTime = 0;
        for(int i = 0; i < recorderInstance.clickCout; i++) {
            totalTime += recorderInstance.clickData[i].clickDuration + 
                        recorderInstance.clickData[i].delayBetweenClicks;
        }
        recorderInstance.averageCPS = (float)(recorderInstance.clickCout * 1000) / totalTime;
    }

    if(clickRecorder.beepOnStart) {
        Beep(1000, 100);
    }

    printf("\nRecorded %d clicks. Average CPS: %.2f\n", recorderInstance.clickCout, recorderInstance.averageCPS);

    // Save config
    if(recorderInstance.clickCout > 0) {
        saveRecordedConfig();
    } else {
        printf("\nNo clicks recorded. Config not saved.\n");
    }

    // Free allocated memory
    free(recorderInstance.clickData);
    recorderInstance.recording = false;
}