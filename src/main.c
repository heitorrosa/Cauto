#include "include/common.h"

#include "config/handler.c"
#include "config/menu.c"

#include <windows.h>

int main() {
    loadDefaultConfigs();

    if(VERSION == "CLI") cliMenu();
}