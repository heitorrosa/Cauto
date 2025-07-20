#ifndef RECORDER_H
#define RECORDER_H

#include "../resources/include.c"
#include "../utils/utils.h"

#define MAX_CLICKS 10000
#define PAUSE_THRESHOLD_MS 200.0  // Fixed threshold: delays over 200ms are filtered

// Simple recorder configuration
typedef struct {
    char bindKey;
    bool beepOnStart;
    bool mcOnly;
} RecorderConfig;

// Simple click structure for recording
typedef struct {
    double duration;    // How long the button was held
    double delay;       // Time until next click
    bool isFiltered;    // Whether this click had a long pause before it
} RecordedClick;

// Recording function - returns encrypted hex config data
char* recordClicks(RecorderConfig* config);

#endif // RECORDER_H
