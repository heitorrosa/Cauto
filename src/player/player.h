#ifndef PLAYER_H
#define PLAYER_H

#include "../resources/include.c"
#include "../utils/utils.h"

// Simple click data structure
typedef struct {
    double duration;    // Click duration in milliseconds
    double delay;       // Delay until next click in milliseconds
} ClickData;

// Simple player configuration
typedef struct {
    char configName[256];
    ClickData* clicks;
    int clickCount;
    double averageCPS;
} PlayerConfig;

// Player state for playback
typedef struct {
    int currentIndex;
    bool isPlaying;
    LARGE_INTEGER lastClickTime;
    LARGE_INTEGER frequency;
} PlayerState;

// Function declarations
void initPlayerState(PlayerState* state);
void resetPlayerState(PlayerState* state);
PlayerConfig* loadPlayerConfig(const char* input);
void freePlayerConfig(PlayerConfig* config);

#endif // PLAYER_H