#ifndef UTILS_H
#define UTILS_H

#include "../resources/include.c"

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