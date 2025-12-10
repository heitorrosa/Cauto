#include "common.h"

clickRecorder_handler recorderInstance;

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

void xorEncrypt(char *data, int length, char key) {
    for(int i = 0; i < length; i++)
        data[i] ^= key;
}

void saveRecordedConfig() {
    char filename[256];
    
    printf("\n\nEnter config name: ");
    scanf_s("%255s", recorderInstance.configName, sizeof(recorderInstance.configName));
    
    strcpy_s(filename, sizeof(filename), recorderInstance.configName);
    strcat_s(filename, sizeof(filename), ".txt");

    FILE *file;
    if(fopen_s(&file, filename, "w") == 0) {
        char buffer[4096] = {0};
        int offset = 0;

        offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%s\n", recorderInstance.configName);
        offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%d,%.2f\n", recorderInstance.clickCout, recorderInstance.averageCPS);
        
        for(int i = 0; i < recorderInstance.clickCout; i++) {
            offset += sprintf_s(buffer + offset, sizeof(buffer) - offset, "%d,%d\n", 
                recorderInstance.clickData[i].clickDuration,
                recorderInstance.clickData[i].delayBetweenClicks);
        }
        
        xorEncrypt(buffer, offset, 0x47);

        for(int i = 0; i < offset; i++)
            fprintf(file, "%02X", (unsigned char)buffer[i]);
        fclose(file);

        printf("\nConfig saved as: %s\n", filename);
    } else
        printf("\nError opening file for writing: %s\n", filename);
}

void clickRecorderHandler() {
    int vKey = charToVKey(clickRecorder.keybind);
    
    printf("\nPress '%c' to start recording...\n", clickRecorder.keybind);

    while(1) {
        if(GetAsyncKeyState(vKey) & 0x8000) {
            robustSleep(0.2);
            break;
        }
        robustSleep(0.001);
    }

    recorderInstance.recording = true;
    recorderInstance.clickCout = 0;
    recorderInstance.clickData = malloc(sizeof(*recorderInstance.clickData) * 1000);

    if(clickRecorder.beepOnStart) Beep(1000, 100);

    printf("Recording clicks... Press '%c' to stop and save.\n", clickRecorder.keybind);

    DWORD lastMouseUpTime = 0;
    DWORD currentClickDownTime = 0;
    bool keybindPressed = false;

    while(recorderInstance.recording) {
        if(GetAsyncKeyState(vKey) & 0x8000) {
            if(!keybindPressed) {
                keybindPressed = true;
            }
        } else
            if(keybindPressed) {
                recorderInstance.recording = false;
                break;
            }

        if(clickRecorder.mcOnly)
            if(GetForegroundWindow() != FindWindowA(MINECRAFT_RECENT, NULL) &&
               GetForegroundWindow() != FindWindowA(MINECRAFT_OLD, NULL) &&
               GetForegroundWindow() != FindWindowA(MINECRAFT_BEDROCK, NULL)) robustSleep(0.001);

        if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if(currentClickDownTime == 0)
                currentClickDownTime = GetTickCount();
        } else {
            if(currentClickDownTime != 0) {
                DWORD clickDownTime = GetTickCount();
                int clickDuration = clickDownTime - currentClickDownTime;
                int delayBetweenClicks = (lastMouseUpTime == 0) ? 0 : (currentClickDownTime - lastMouseUpTime);

                if(clickDuration <= clickRecorder.clickDuration_threshold &&
                   delayBetweenClicks <= clickRecorder.delayBetweenClicks_threashold) {

                    recorderInstance.clickData[recorderInstance.clickCout].clickDuration = clickDuration;
                    recorderInstance.clickData[recorderInstance.clickCout].delayBetweenClicks = delayBetweenClicks;
                    recorderInstance.clickCout++;
                }

                lastMouseUpTime = clickDownTime;
                currentClickDownTime = 0;
            }
        }

        robustSleep(0.001);
    }

    if(recorderInstance.clickCout > 0) {
        int totalTime = 0;
        for(int i = 0; i < recorderInstance.clickCout; i++) {
            totalTime += recorderInstance.clickData[i].clickDuration + 
                        recorderInstance.clickData[i].delayBetweenClicks;
        }
        recorderInstance.averageCPS = (float)(recorderInstance.clickCout * 1000) / totalTime;
    }

    if(clickRecorder.beepOnStart) Beep(1000, 100);

    printf("\nRecorded %d clicks. Average CPS: %.2f\n", recorderInstance.clickCout, recorderInstance.averageCPS);

    if(recorderInstance.clickCout > 0)
        saveRecordedConfig();
    else
        printf("\nNo clicks recorded. Config not saved.\n");

    free(recorderInstance.clickData);
    recorderInstance.recording = false;
}