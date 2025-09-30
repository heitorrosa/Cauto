#ifndef CLICKSOUNDS_H
#define CLICKSOUNDS_H

#include "common.h"

typedef struct {
    char *data;
    size_t size;
    char *path;
} Sound;

typedef struct {
    Sound *soundData;
    int fileCount;
} ClickSounds;

// Public function declarations
void selectClickSounds();
void playClickSounds();

#endif // CLICKSOUNDS_H