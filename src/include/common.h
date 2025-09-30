#ifndef COMMON_H
#define COMMON_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <ctype.h>

#include <libwebsockets.h>

#include "config.h"
extern leftClicker_config leftClicker;
extern clickPlayer_config clickPlayer;
extern clickRecorder_config clickRecorder;
extern globalSettings_config globalSettings;

// Include headers
#include "header/utils.h"
#include "header/clickSounds.h"
#include "header/leftClicker.h"
#include "header/clickPlayer.h"
#include "header/clickRecorder.h"

#include "header/handler.h"
#include "header/menu.h"

extern ClickRandomizer *randomizer;
extern ClickSounds clickSounds;

// Definitions
#define MINECRAFT_RECENT "GLFW30"
#define MINECRAFT_OLD    "LWJGL"
#define MINECRAFT_BEDROCK "ApplicationFrameWindow"

#endif /* COMMON_H */