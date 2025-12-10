#ifndef CLICKPLAYER_H
#define CLICKPLAYER_H

#include "common.h"

void loadConfigFromFile(clickPlayer_config *config);
void loadConfigFromClipboard(clickPlayer_config *config);
void loadConfigFromString(const char *hexStr, clickPlayer_config *config);

void clickPlayerHandler(void);
#endif /* CLICKPLAYER_H */