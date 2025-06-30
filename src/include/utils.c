#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <math.h> 

#include "utils.h"

void initGlobalConfig(globalConfig *config) {
    config->leftActive = false;
    config->playerActive = false;

    config->mcOnly = true;
    config->clickInventory = false;
    config->breakBlocks = false;
    config->soundClicks = true;
}

bool cursorVisible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;
    
    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void sendLeftClickDown(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    PostMessageA(GetForegroundWindow(), down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}