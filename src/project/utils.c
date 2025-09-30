#include "common.h"

// Alternative function to system("cls") which creates API calls that can get tracked easily
void clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        COORD homeCoords = {0, 0};
        DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y, count;
        
        FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
        SetConsoleCursorPosition(hConsole, homeCoords);
    }
}

// Function to sendClicks to a specific window using PostMessageA (not effective in Bedrock)
void sendPostMessageA(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    PostMessageA(GetForegroundWindow(), down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}

// Function to check for cursor visibility in JVM based applications
bool cursorVisible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    if (!GetCursorInfo(&ci)) return false;
    return (ci.flags & CURSOR_SHOWING) != 0;
}

// Function to check for cursor visibility in general applications (Bedrock compatible)
int cursorVisibleSystem() {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci)) {
        return false;
    }
    
    int systemVisible = (ci.flags & CURSOR_SHOWING) != 0;
    int handleValid = ci.hCursor != NULL;
    
    return systemVisible && handleValid;
}