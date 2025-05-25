#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "utils.h"

void init_config(configCauto *config) {
    config->active = true;
    config->mcOnly = true;
    config->clickInventory = false;
    config->breakBlocks = false;

    config->inputCPS = 14;
    config->minDurationClick = 22;
    config->maxDurationClick = 33;

    config->dropChance = 50;
    config->spikeChance = 50;
    config->dropCPS = 3;
    config->spikeCPS = 2;
}

configCauto config;

float randomization(float min, float max) {
    static unsigned int seed = 0;
    if (!seed) seed = GetTickCount() ^ GetCurrentProcessId();
    seed = (214013 * seed + 2531011);
    return min + (seed >> 16) * (1.0f / 65535.0f) * (max - min);
}

bool visibleCursor(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;

    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void sendClick(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    HWND window = GetForegroundWindow();

    // FindWindowA(NULL, "javaw");
    //GetForegroundWindow();
    PostMessage(window, down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}