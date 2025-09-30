#ifndef CLICKRECORDER_H
#define CLICKRECORDER_H

#include "common.h"

typedef struct {
    bool recording;

    int clickCout;
    int averageCPS;
    struct {
        int clickDuration;
        int delayBetweenClicks;
    } *clickData;
} clickRecorder_Handler;

#endif /* CLICKRECORDER_H */
