#ifndef RECORDER_H
#define RECORDER_H

#include "../resources/include.c"
#include "../utils/utils.h"

typedef struct {
    int unifiedClickCount;
    int doubleClicks;
    int totalClicks;
} clickRecorder;

typedef struct {
    double duration;    // How long the button was held
    double delay;       // Time until next click
} UnifiedClick;

#define MAX_CLICKS 1000000

int recordClicks(void);
void xor_encrypt_decrypt(const char* input, char* output, int length);
void binary_to_hex(const char* binary, char* hex, int length);
int hex_to_binary(const char* hex, char* binary);

#endif // RECORDER_H
