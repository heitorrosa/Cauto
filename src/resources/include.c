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

#include <libwebsockets.h>

// Config limits for large files
#define MAX_CLICKS 1000000      // 1 million clicks max
#define MAX_CONFIG_SIZE 50000000 // 50MB max config size