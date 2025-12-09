#include "common.h"

void loadDefaultConfigs() {
    //
    // Left Clicker
    //
    leftClicker.enabled = true;

    leftClicker.cps = 13;
    leftClicker.minDuration = 22;
    leftClicker.maxDuration = 30;

    leftClicker.dropChance = 50;
    leftClicker.cpsDrop = 3;

    leftClicker.spikeChance = 50;
    leftClicker.cpsSpike = 2;

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
    
    globalSettings.jitterChance = 60;
    globalSettings.jitterX = 0;
    globalSettings.jitterY = 0;

    globalSettings.clickSounds = false;
}