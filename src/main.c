#include "include/common.h"

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
            if(globalSettings.clickSounds && clickSounds.fileCount > 0) playClickSounds();
        } else {
            PlaySoundA(NULL, NULL, SND_PURGE);
        }

        Sleep(1);
    }
}