#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

#define VERSION CLI // WEB or CLI

typedef struct {
    bool enabled = true;

    int cps = 13;
    int minInterval = 22; // 22ms
    int maxInterval = 30; // 30ms

    int dropChance = 50; // 50%
    int cpsDrop = 3;
    int spikeChance = 50; // 50%
    int cpsSpike = 2;
} leftClicker_Config;

typedef struct {
    bool enabled = false;

    char configName;
    int clickCount;
    int averageCPS;

    // clicks* dataframe

} clickPlayer_Config;

typedef struct {
    bool recording = false;
    bool beepOnStart = true;
    bool mcOnly = true;

    char keybind;
    int threashold = 250; // 250ms
} clickRecorder_Config;

typedef struct {
    bool mcOnly = true;
    bool breakBlocks = false;
    bool clickInventory = false;

    bool clickSounds = false;
} globalSettings_Config;

#endif /* CONFIG_H */
