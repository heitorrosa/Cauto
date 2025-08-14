#ifndef UTILS_H
#define UTILS_H

#include "../common/common.h"

typedef struct {
    char* data;
    DWORD size;
} WavFile;

typedef struct {
    WavFile* files;
    int count;
    int capacity;
} WavCollection;

typedef struct globalConfig {
    bool leftActive;
    bool playerActive;

    bool mcOnly;
    bool clickInventory;
    bool breakBlocks;
    bool soundClicks;
} globalConfig;

void initGlobalConfig(globalConfig *config);
bool cursorVisible(void);
void sendLeftClickDown(bool down);
bool openWavFileDialog(WavCollection* collection);
void freeWavCollection(WavCollection* collection);
char* getRandomWavData(WavCollection* collection, DWORD* size);
void clearScreen(void);
void precisionSleep(double milliseconds);
char* openConfigFileDialog(void);
char* getClipboardData(void);

#endif // UTILS_H