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

void init_config(configCauto *config);
float randomizar(float min, float max);
bool cursorVisivel(void);
void enviarClique(bool down);

#endif // UTILS_H