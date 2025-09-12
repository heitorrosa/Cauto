#ifndef CLICKRECORDER_H
#define CLICKRECORDER_H

#include "../../include/common.h"
#include "../../include/config.h"

clickRecorder_Config recorder_config;
typedef struct {
    bool recording = false;

    int clickCout = 0;
    int averageCPS = 0;
    struct {
        int clickDuration;
        int delayBetweenClicks;
    } *clickData;
} clickRecorder_Handler;

#endif /* CLICKRECORDER_H */
