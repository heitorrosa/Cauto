#ifndef UTILS_H
#define UTILS_H

#include "common.h"

// Public function declarations
void clearScreen();
void robustSleep(double seconds);
void sendPostMessageA(bool down);

bool cursorVisible(void);
int cursorVisibleSystem();

#endif // UTILS_H