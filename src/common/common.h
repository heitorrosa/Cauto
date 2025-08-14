#ifndef COMMON_H
#define COMMON_H

// System and Windows headers
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

// Buffer sizes
#define CLIPBOARD_BUFFER_MAX (10 * 1024 * 1024) // 10 MB
#define CONFIG_NAME_MAX 256
#define RANDOM_NAME_LEN 32
#define DIALOG_MULTISELECT_BUFFER 32768
#define MACHINE_GUID_BUFFER 256
#define HWID_STRING_MAX 64
#define LINE_BUFFER_MAX 256

// Windows class names
#define MINECRAFT_CLASS_RECENT "GLFW30"
#define MINECRAFT_CLASS_OLD    "LWJGL"
#define MINECRAFT_CLASS_BEDROCK "ApplicationFrameWindow"

// Timing and thresholds
#define MIN_CLICK_DURATION_MS 10.0f
#define MIN_DELAY_AFTER_CLICK_MS 5.0f
#define DEFAULT_PAUSE_THRESHOLD_MS 200.0f
#define FALLBACK_AVG_DELAY_MS 80.0f
#define IDLE_UPDATE_INTERVAL_MS 100ULL
#define INACTIVITY_THRESHOLD_MS 2000ULL
#define IDLE_SLEEP_MS 1UL
#define RECORD_START_POLL_MS 10UL
#define RECORD_LOOP_SLEEP_MS 1UL
#define HUMAN_ACTIVE_WINDOW_MS 1000UL
#define CLICKER_MIN_INTERVAL_MS 12.0f

// Ranges
#define PERCENT_MIN 0.0f
#define PERCENT_MAX 100.0f

// Limits
#define MAX_CLICKS 1000000       // 1M
#define MAX_CONFIG_SIZE (50 * 1024 * 1024) // 50MB
#define PLAYER_MIN_HEX_LEN 100

// Other
#define WAV_INIT_CAPACITY 10
#define BEEP_START_FREQ 800
#define BEEP_END_FREQ 600
#define BEEP_DURATION_MS 200
#define TWO_PI 6.28318530718f

// Utility macro
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

#endif // COMMON_H
