#include "include/common.h"

#include "project/utils.c"
#include "project/clickSounds.c"

#include "config/menu.c"
#include "config/handler.c"

int main() {
    srand(time(NULL));

    loadDefaultConfigs();

    if(strcmp(VERSION, "CLI") == 0) cliMenu();

    while(1) {
        if(strcmp(VERSION, "CLI") == 0) {
            if (kbhit()) {
                char menuKeybind = getch();
                if (menuKeybind == 'm' || menuKeybind == 'M') cliMenu();
            }
        }

        if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if(leftClicker.enabled && leftClicker.cps > 0) {
                sendPostMessageA(true);
                sendPostMessageA(false);
                
                Sleep(100);
            }

            if(clickPlayer.enabled && clickPlayer.clickCout > 0) printf("click player");

            if(globalSettings.jitterX > 0 || globalSettings.jitterY > 0) printf("jitter");

            if(globalSettings.clickSounds && clickSounds.fileCount > 0) playClickSounds();
        } else {
            PlaySoundA(NULL, NULL, SND_PURGE);
        }

        Sleep(1);
    }
}