#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

#define VERSION "CLI" // WEB or CLI

/*
@enable Boolean to check if the leftClicker is enabled

@cps Amount of Clicks Per Second (CPS)

@minInterval Minimal Invterval between Clicks (ms)
@maxInterval Max Interval between Clicks (ms)
--# Make the clicks flash more human in the Keystrokes mod for example and helps in randomization

@dropChance Chance for a Drop in the CPS, helping in randomization (%)
@cpsDrop The amount of CPS to drop when it is in its hit chance

@spikeChance Chance for a Spike in the CPS, helping in randomization (%)
@cpsSpike The amount of CPS to spike when it is in its hit chance
*/
typedef struct {
    bool enabled; // true

    int cps; // 13cps
    int minDuration; // 22ms
    int maxDuration; // 30ms

    int dropChance; // 50%
    int cpsDrop; // 3

    int spikeChance; // 50%
    int cpsSpike; // 2
} leftClicker_config;

/*
@enable boolean to check if the clicker is enabled

@configName The name of the config in the loaded config

@clickCount The amount of clicks in the config
@averageCPS The average CPS for the config
@cickData A struct for the things related with clicks in the config
*/
typedef struct {
    bool enabled; // false

    char *configName;
    int clickCout;
    int averageCPS;
    struct {
        int clickDuration;
        int delayBetweenClicks;
    } *clickData;
} clickPlayer_config;

/*
@beepOnStart Emit a system-level beep to indicate if the program is recording your clicks or not
@mcOnly Boolean to verifies if the recorder should only work in instances of the "Minecraft" environment

@keybind The assigned keybind in which the recorder will be toggled
@threashold The maximum allowed pause for the recorder, if it goes above this threashold in ms, the clickIndex will be discarted (ms)
*/
typedef struct {
    bool beepOnStart; // true
    bool mcOnly; // true

    char keybind; // B
    int delayBetweenClicks_threashold; // 250ms
    int clickDuration_threshold; // 100ms
} clickRecorder_config;

/*
@mcOnly Boolean to verifies if the clicker should only work in instances of the "Minecraft" environment
@breakBlocks Boolean to verify if the clicker should use the BreakBlocks function
@clickInventory Boolean to verify if the clicker should work when the player is in its inventory (cursor visible)

@jitterChance The change of the jitter state to be triggereed (%)
@jitterY The amount of pixels in the Y axis that the program will move (px)
@jitterX The amount of pixels in the X axis that the program will move (px)

@clickSounds Boolean to verify if the Click Sounds feature is enabled, if will be enabled once its verified that an audio file was loaded in the program's memory.
*/
typedef struct {
    bool mcOnly; // true
    bool breakBlocks; // false
    bool clickInventory; // false

    int jitterChance; // 60%
    int jitterY; // 0px
    int jitterX; // 0px

    bool clickSounds; // false
} globalSettings_config;

#endif /* CONFIG_H */
