#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

typedef struct globalConfig {
    bool leftActive;
    bool playerActive;

    bool mcOnly;
    bool clickInventory;
    bool breakBlocks;
    bool soundClicks;
} globalConfig;


void initGlobalConfig(globalConfig *config);
bool cursorVisible(void);
void sendLeftClickDown(bool down);

#endif // UTILS_H