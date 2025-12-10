#include "common.h"

leftClicker_config leftClicker;
clickPlayer_config clickPlayer = {0};
clickRecorder_config clickRecorder;
globalSettings_config globalSettings;
ClickRandomizer *randomizer = NULL;
ClickSounds clickSounds = { NULL, 0 };

int main() {
    timeBeginPeriod(1);
    srand(time(NULL));

    ClickRandomizer clickRandomizer;
    randomizer = &clickRandomizer;
    initRandomizer();
    loadDefaultConfigs();

    clickPlayer.configName = malloc(256);

    if(strcmp(VERSION, "CLI") == 0) cliMenu();
    if(strcmp(VERSION, "WEB") == 0) printf("websocket");

    while(1) {
        if(strcmp(VERSION, "CLI") == 0)
            if (kbhit()) {
                char menuKeybind = getch();
                if (menuKeybind == 'm' || menuKeybind == 'M') cliMenu();
            }

        if((GetForegroundWindow() != FindWindowA(MINECRAFT_RECENT, NULL) ||
            GetForegroundWindow() != FindWindowA(MINECRAFT_OLD, NULL) ||
            GetForegroundWindow() != FindWindowA(MINECRAFT_BEDROCK, NULL)) && globalSettings.mcOnly) robustSleep(0.001);

        if(GetAsyncKeyState(VK_LBUTTON) & 0x8000 && (globalSettings.clickInventory || !cursorVisible())) {
            if(leftClicker.enabled && leftClicker.cps > 0) leftClickerHandler();
            if(clickPlayer.enabled && clickPlayer.clickCout > 0) clickPlayerHandler();

            if(globalSettings.jitterX > 0 || globalSettings.jitterY > 0) mouseJitterHandler();
            if(globalSettings.clickSounds && clickSounds.fileCount > 0) playClickSounds();
        } else
            PlaySoundA(NULL, NULL, SND_PURGE);

        robustSleep(0.001);
    }
}