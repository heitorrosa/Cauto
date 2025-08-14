#ifndef PLAYER_H
#define PLAYER_H

#include "../common/common.h"
#include "../utils/utils.h"

// Simple click data structure
typedef struct {
    double duration;    // Click duration in milliseconds
    double delay;       // Delay until next click in milliseconds
} ClickData;

// Simple player configuration
typedef struct {
    char configName[CONFIG_NAME_MAX];
    ClickData* clicks;
    int clickCount;
    double averageCPS;
} PlayerConfig;

// Player state for playback
typedef struct {
    int currentIndex;
    bool isPlaying;
    bool wasPlayingLastFrame;  // Track if we were playing in the previous frame
    LARGE_INTEGER lastClickTime;
    LARGE_INTEGER frequency;
} PlayerState;

// Function declarations
void initPlayerState(PlayerState* state);
void resetPlayerState(PlayerState* state);
void setRandomStartPosition(PlayerState* state, int maxPosition);
PlayerConfig* loadPlayerConfig(const char* input);
void freePlayerConfig(PlayerConfig* config);

#endif // PLAYER_H