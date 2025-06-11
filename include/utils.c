#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "utils.h"

void init_config(configCauto *config) {
    config->active = true;
    config->mcOnly = true;
    config->clickInventory = false;
    config->breakBlocks = true;

    config->inputCPS = 13;
    config->minDurationClick = 22;
    config->maxDurationClick = 33;

    config->dropChance = 50;
    config->spikeChance = 50;
    config->dropCPS = 3;
    config->spikeCPS = 2;
}

void init_random_state(RandomState *state) {
    state->remainingTicks = 0;
    state->cpsModifier = 0;
    state->randomSeed = GetTickCount() ^ GetCurrentProcessId();
}

configCauto config;

// Original randomization function - kept for compatibility
float randomization(float min, float max) {
    static unsigned int seed = 0;
    if (!seed) seed = GetTickCount() ^ GetCurrentProcessId();
    seed = (214013 * seed + 2531011);
    return min + (seed >> 16) * (1.0f / 65535.0f) * (max - min);
}

// Enhanced randomization with better entropy
float enhanced_randomization(float min, float max, RandomState *state) {
    // Multiple random algorithms for better distribution
    state->randomSeed = (1103515245 * state->randomSeed + 12345) & 0x7fffffff;
    float rand1 = (float)state->randomSeed / 0x7fffffff;
    
    // Add XorShift for additional entropy
    static unsigned int xor_seed = 0;
    if (!xor_seed) xor_seed = GetTickCount() + 1;
    xor_seed ^= xor_seed << 13;
    xor_seed ^= xor_seed >> 17;
    xor_seed ^= xor_seed << 5;
    float rand2 = (float)(xor_seed & 0xFFFF) / 0xFFFF;
    
    // Combine both randoms with slight bias toward first
    float combined = (rand1 * 0.75f + rand2 * 0.25f);
    
    return min + combined * (max - min);
}

// Calculate CPS with burst sequences using only Spikes and Drops
float calculate_cps_with_bursts(configCauto *config, RandomState *state) {
    float finalCPS = (float)config->inputCPS;
    
    // Check if we're in a burst sequence
    if (state->remainingTicks > 0) {
        // Continue the current burst/drop
        finalCPS += state->cpsModifier;
        state->remainingTicks--;
        
        // Gradually reduce the effect over time for more natural feel
        if (state->remainingTicks > 0) {
            state->cpsModifier = (int)(state->cpsModifier * 0.9f);
        }
    } else {
        // Check for new burst/drop sequence
        float randomChance = enhanced_randomization(0, 100, state);
        
        if (randomChance < config->dropChance) {
            // Start drop sequence (2-4 clicks duration)
            state->cpsModifier = -config->dropCPS;
            state->remainingTicks = (int)enhanced_randomization(2, 5, state);
            finalCPS += state->cpsModifier;
        } else if (randomChance < config->dropChance + config->spikeChance) {
            // Start spike sequence (2-3 clicks duration)
            state->cpsModifier = config->spikeCPS;
            state->remainingTicks = (int)enhanced_randomization(2, 4, state);
            finalCPS += state->cpsModifier;
        } else {
            // Add small natural variation (±0.5 CPS) even when no burst
            float naturalVariation = enhanced_randomization(-0.5f, 0.5f, state);
            finalCPS += naturalVariation;
        }
    }
    
    // Ensure minimum CPS
    if (finalCPS < 1.0f) finalCPS = 1.0f;
    
    return finalCPS;
}

bool visibleCursor(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;
    
    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void sendClick(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    HWND window = GetForegroundWindow();

    PostMessage(window, down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}