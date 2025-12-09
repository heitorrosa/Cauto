#ifndef CLICKRECORDER_H
#define CLICKRECORDER_H

#include "common.h"

typedef struct {
    bool recording;

    char configName[256];
    int clickCout;
    float averageCPS;
    struct {
        int clickDuration;
        int delayBetweenClicks;
    } *clickData;
} clickRecorder_handler;

void clickRecorderHandler(void);

#endif /* CLICKRECORDER_H */
