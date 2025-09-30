#ifndef LEFTCLICKER_H
#define LEFTCLICKER_H

#include "common.h"

typedef struct {
    // Random state
    unsigned int chacha_state[16];
    unsigned int counter;
    unsigned long long entropy;
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastClickTime;
    
    // Human factors
    float concentration;
    float fatigue;
    float rhythm;
    int microPauseCounter;
    
    // Anti-detection enhancements
    float exhaustionLevel;
    int outlierForced;
    float kurtosisCounter;
    bool needsBreak;
    DWORD lastBreakTime;
    
    // Recovery system
    DWORD lastActiveTime;
    bool isRecovering;
    float recoveryRate;
    bool wasInactive;
    
    // Session tracking
    DWORD sessionStart;
    int clickCount;
    
    // Pattern prevention
    float recentIntervals[16];
    int intervalIndex;
    
    // Burst/drop system
    int modifierRemaining;
    int cpsModifier;
    
    // Natural clustering
    bool inCluster;
    int clusterRemaining;
    float clusterMultiplier;
    
    // Advanced state
    float baseVariation;
    float momentaryBias;
} ClickRandomizer;

void initRandomizer(void);
float getRandomInterval(void);
float getRandomDuration(void);
void leftClickerHandler(void);

#endif /* LEFTCLICKER_H */