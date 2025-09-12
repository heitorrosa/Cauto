#include "../include/common.h"
#include "../include/config.h"

leftClicker_config leftClicker;
clickPlayer_config clickPlayer;
clickRecorder_config clickRecorder;
globalSettings_config globalSettings;

void loadDefaultConfigs() {
    //
    // Left Clicker
    //
    leftClicker.enabled = true;

    leftClicker.cps = 13;
    leftClicker.minInterval = 22;
    leftClicker.maxInterval = 30;

    leftClicker.dropChance = 50;
    leftClicker.cpsDrop = 3;

    leftClicker.spikeChance = 50;
    leftClicker.cpsSpike = 2;

    leftClicker.jitterX = 0;
    leftClicker.jitterY = 0;


    //
    // Click Recorder
    //
    clickPlayer.enabled = false;


    //
    // Click Recorder
    //
    clickRecorder.beepOnStart = true;
    clickRecorder.mcOnly = true;
    clickRecorder.keybind = 'B';

    clickRecorder.clickDuration_threshold = 250;
    clickRecorder.delayBetweenClicks_threashold = 100;


    //
    // Global Settings
    //
    globalSettings.mcOnly = false;
    globalSettings.breakBlocks = false;
    globalSettings.clickInventory = false;
    
    globalSettings.clickSounds = false;
}