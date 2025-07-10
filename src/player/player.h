#ifndef PLAYER_H
#define PLAYER_H

#include "../resources/include.c"
#include "../utils/utils.h"

typedef struct {
    int id;
    double duration;
    double delay;
} ParsedClick;

typedef struct {
    char configName[256];
    int totalClicks;
    int doubleClicks;
    int unifiedClicks;
    double averageCPS;
    ParsedClick* clicks;
    int clickCount;
} PlayerConfig;


PlayerConfig* getPlayerConfig(bool getRawConfig, const char* rawConfigData);
void freePlayerConfig(PlayerConfig* config);
int playClicks(const char* input);
int listAndDecodeConfigs(void);

#endif // PLAYER_H