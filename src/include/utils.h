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

// Enhanced state for more sophisticated burst sequences
typedef struct {
    int remainingTicks;
    int cpsModifier;
    unsigned int randomSeed;
    unsigned int xorSeed;
    unsigned int performanceSeed;
    float jitterAccumulator;
    int burstCooldown;
    float lastCpsValue;
} RandomState;

// Preset configurations
typedef struct {
    int cps;
    float dropChance;
    float spikeChance;
    int dropCPS;
    int spikeCPS;
} CPSPreset;

void init_config(configCauto *config);
void init_random_state(RandomState *state);
float randomization(float min, float max);
float enhanced_randomization(float min, float max, RandomState *state);
float calculate_cps_with_bursts(configCauto *config, RandomState *state);
bool visibleCursor(void);
void sendClick(bool down);
void apply_cps_preset(configCauto *config, int targetCPS);
void display_recommended_settings(void);

#endif // UTILS_H