#include "common.h"

typedef struct {
    POINT origin;
    POINT destination;
    POINT current;
} JitterState;

static JitterState jitterState = {{0, 0}, {0, 0}, {0, 0}};

static unsigned int advancedRandom(void) {
    unsigned int *s = randomizer->chacha_state;
    s[12] = randomizer->counter++;
    
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 16) | (s[12] >> 16);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 12) | (s[4] >> 20);
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 8) | (s[12] >> 24);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 7) | (s[4] >> 25);
    
    return s[0] ^ s[4] ^ s[8] ^ s[12];
}

static float randomRange(float min, float max) {
    float f = (float)(advancedRandom() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
    return min + f * (max - min);
}

static int randomInt(int min, int max) {
    return min + (advancedRandom() % (max - min + 1));
}

static void moveMouseX(int delta) {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + delta, p.y);
    jitterState.current.x += delta;
}

static void moveMouseY(int delta) {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x, p.y + delta);
    jitterState.current.y += delta;
}

void mouseJitterHandler(void) {
    if (globalSettings.jitterX <= 0 && globalSettings.jitterY <= 0) {
        return;
    }
    
    if (jitterState.current.x == jitterState.destination.x && 
        jitterState.current.y == jitterState.destination.y) {
        
        if (randomInt(0, 100) < globalSettings.jitterChance) {
            GetCursorPos(&jitterState.origin);
            jitterState.current = jitterState.origin;
            jitterState.destination.x = jitterState.origin.x + randomInt(-globalSettings.jitterX, globalSettings.jitterX);
            jitterState.destination.y = jitterState.origin.y + randomInt(-globalSettings.jitterY, globalSettings.jitterY);
        }
        return;
    }
    
    if (jitterState.current.x < jitterState.destination.x)
        moveMouseX(1);
    else if (jitterState.current.x > jitterState.destination.x)
        moveMouseX(-1);

    
    if (jitterState.current.y < jitterState.destination.y)
        moveMouseY(1);
    else if (jitterState.current.y > jitterState.destination.y)
        moveMouseY(-1);
    
    robustSleep(randomRange(0.003, 0.01));
}