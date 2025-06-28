#ifndef CLICKER_H
#define CLICKER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "../include/utils.h"

typedef struct {
    int inputCPS;
    float minDurationClick;
    float maxDurationClick;

    float dropChance;
    float spikeChance;
    int dropCPS;
    int spikeCPS;
} clickerConfig;

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

float randomization(float min, float max, RandomState *state);
float gaussianRandom(GaussianState *gauss, RandomState *state);
float adaptiveRandomization(float base, float variance, RandomState *state);
float cpsWithBursts(clickerConfig *clicker, RandomState *state);
float calculateJitter(RandomState *state);
void initRandomState(RandomState *state);
void initClickerConfig(clickerConfig *clicker);

#endif // CLICKER_H