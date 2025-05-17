#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

// Configurações do AutoClicker
typedef struct {
    bool ativo = true;
    bool clicarInventario = false;

    int inputCPS = 14;
    int durMinClique = 22;
    int durMaxClique = 30;

    int chanceDrop = 30;
    int chanceSpike = 30;
    int cpsDrop = 2;
    int cpsSpike = 1;

    int variacaoClique = 10;

} configCauto;


float randomizar(float min, float max) {
    static unsigned int seed = 0;
    if (!seed) seed = GetTickCount() ^ GetCurrentProcessId();
    seed = (214013 * seed + 2531011);
    return min + (seed >> 16) * (1.0f / 65535.0f) * (max - min);
}

bool cursorVisivel(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;

    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void enviarClique(bool down) {
    POINT pos;
    GetCursorPos(&pos);
    HWND window = GetForegroundWindow();
    PostMessage(window, down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}