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

// Simple state for burst sequences
typedef struct {
    int remainingTicks;
    int cpsModifier;
    unsigned int randomSeed;
} RandomState;

void init_config(configCauto *config);
void init_random_state(RandomState *state);
float randomization(float min, float max);
float enhanced_randomization(float min, float max, RandomState *state);
float calculate_cps_with_bursts(configCauto *config, RandomState *state);
bool visibleCursor(void);
void sendClick(bool down);

#endif // UTILS_H