#ifndef RECORDER_H
#define RECORDER_H

#include "../resources/include.c"
#include "../utils/utils.h"

#define MAX_CLICKS 1000000

typedef struct {
    int unifiedClickCount;
    int doubleClicks;
    int totalClicks;

    char beepOnStart;
    char mcOnly;
    char bindKey;
} clickRecorder;

typedef struct {
    int id;             // Unique identifier for this click
    double duration;    // How long the button was held
    double delay;       // Time until next click
    bool hasDelay;      // Whether delay has been set
} UnifiedClick;

// Recording function - returns encrypted hex config data, keeps all features
char* recordClicks(clickRecorder* recorder);

#endif // RECORDER_H
