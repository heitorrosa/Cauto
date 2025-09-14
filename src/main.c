#include "include/common.h"

#include "config/handler.c"
#include "config/menu.c"

#include <windows.h>

int main() {
    loadDefaultConfigs();

    if(VERSION == "CLI") cliMenu();

    while(1) {
        if (kbhit()) {
            char menuKeybind = getch();
            if (menuKeybind == 'm' || menuKeybind == 'M') cliMenu();
        }

        Sleep(1);
    }
}