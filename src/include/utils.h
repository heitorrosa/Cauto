#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

typedef struct configCauto {
    bool active;
    bool mcOnly;
    bool clickInventory;
    bool breakBlocks;

    int inputCPS;
    float minDurationClick;
    float maxDurationClick;

    float dropChance;
    float spikeChance;
    int dropCPS;
    int spikeCPS;
} configCauto;

// Enhanced randomization state with multiple entropy sources
typedef struct {
    // Burst sequence state
    int remainingTicks;
    int cpsModifier;
    
    // Multiple PRNG states for different purposes
    unsigned int linearSeed;    // Linear congruential for basic randomness
    unsigned int xorshiftSeed;  // XorShift for better distribution
    unsigned int lfsr;          // Linear feedback shift register
    
    // Timing analysis and adaptation
    LARGE_INTEGER lastClickTime;
    LARGE_INTEGER frequency;
    float clickHistory[32];     // Track recent click intervals
    int historyIndex;
    
    // Natural variation state
    float fatigueLevel;         // Simulates hand fatigue over time
    int consistentClickCount;   // Track how many consistent clicks
    DWORD sessionStartTime;     // For session-based patterns
} RandomState;

// Gaussian distribution parameters
typedef struct {
    float mean;
    float stddev;
    bool hasSpare;
    float spare;
} GaussianState;

void init_config(configCauto *config);
void init_randomState(RandomState *state);
float randomization(float min, float max, RandomState *state);
float gaussianRandom(GaussianState *gauss, RandomState *state);
float adaptiveRandomization(float base, float variance, RandomState *state);
float cpsWithBursts(configCauto *config, RandomState *state);
float calculateJitter(RandomState *state);
bool visibleCursor(void);
void sendClick(bool down);

#endif // UTILS_H