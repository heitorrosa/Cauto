#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

    config->dropChance = 45;
    config->spikeChance = 20;
    config->dropCPS = 3;
    config->spikeCPS = 2;
}

void init_random_state(RandomState *state) {
    state->remainingTicks = 0;
    state->cpsModifier = 0;
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    state->randomSeed = GetTickCount() ^ GetCurrentProcessId();
    state->xorSeed = (unsigned int)perfCounter.LowPart + 1;
    state->performanceSeed = (unsigned int)perfCounter.HighPart ^ GetCurrentThreadId();
    state->jitterAccumulator = 0.0f;
    state->burstCooldown = 0;
    state->lastCpsValue = 0.0f;
}

// Original randomization function - kept for compatibility
float randomization(float min, float max) {
    static unsigned int seed = 0;
    if (!seed) seed = GetTickCount() ^ GetCurrentProcessId();
    seed = (214013 * seed + 2531011);
    return min + (seed >> 16) * (1.0f / 65535.0f) * (max - min);
}

// Enhanced randomization with multiple entropy sources and natural jitter
float enhanced_randomization(float min, float max, RandomState *state) {
    // Primary LCG
    state->randomSeed = (1103515245 * state->randomSeed + 12345) & 0x7fffffff;
    float rand1 = (float)state->randomSeed / 0x7fffffff;
    
    // XorShift32
    state->xorSeed ^= state->xorSeed << 13;
    state->xorSeed ^= state->xorSeed >> 17;
    state->xorSeed ^= state->xorSeed << 5;
    float rand2 = (float)(state->xorSeed & 0xFFFFFF) / 0xFFFFFF;
    
    // Performance counter based randomness
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    state->performanceSeed = (state->performanceSeed * 69069 + 1) ^ (unsigned int)perfCounter.LowPart;
    float rand3 = (float)(state->performanceSeed & 0xFFF) / 0xFFF;
    
    // Combine with weighted distribution for more natural feel
    float combined = (rand1 * 0.5f + rand2 * 0.35f + rand3 * 0.15f);
    
    // Add slight bias toward center values (more human-like)
    combined = combined * combined * (3.0f - 2.0f * combined); // Smoothstep function
    
    return min + combined * (max - min);
}

// Calculate CPS with enhanced burst sequences and natural jitter
float calculate_cps_with_bursts(configCauto *config, RandomState *state) {
    float baseCPS = (float)config->inputCPS;
    float finalCPS = baseCPS;
    
    // Add subtle continuous jitter (±0.3 CPS) for natural feel
    float microJitter = enhanced_randomization(-0.3f, 0.3f, state);
    state->jitterAccumulator = state->jitterAccumulator * 0.8f + microJitter * 0.2f; // Smooth the jitter
    finalCPS += state->jitterAccumulator;
    
    // Handle burst cooldown
    if (state->burstCooldown > 0) {
        state->burstCooldown--;
    }
    
    // Check if we're in a burst sequence
    if (state->remainingTicks > 0) {
        // Continue the current burst/drop with gradual decay
        float decayFactor = (float)state->remainingTicks / (state->remainingTicks + 2.0f);
        int adjustedModifier = (int)(state->cpsModifier * decayFactor);
        finalCPS += adjustedModifier;
        state->remainingTicks--;
        
        if (state->remainingTicks == 0) {
            // Set cooldown based on burst intensity
            state->burstCooldown = abs(state->cpsModifier) * 2 + (int)enhanced_randomization(3, 8, state);
        }
    } else if (state->burstCooldown == 0) {
        // Check for new burst/drop sequence with adaptive probabilities
        float adaptedDropChance = config->dropChance;
        float adaptedSpikeChance = config->spikeChance;
        
        // Slightly adjust chances based on base CPS for more realistic patterns
        if (baseCPS > 15) {
            adaptedDropChance *= 1.1f; // Higher CPS = slightly more drops
            adaptedSpikeChance *= 0.9f; // But fewer spikes
        }
        
        float randomChance = enhanced_randomization(0, 100, state);
        
        if (randomChance < adaptedDropChance) {
            // Start drop sequence with duration based on base CPS
            state->cpsModifier = -config->dropCPS;
            int minDuration = baseCPS > 15 ? 2 : 3;
            int maxDuration = baseCPS > 15 ? 4 : 6;
            state->remainingTicks = (int)enhanced_randomization(minDuration, maxDuration + 1, state);
            finalCPS += state->cpsModifier;
        } else if (randomChance < adaptedDropChance + adaptedSpikeChance) {
            // Start spike sequence with shorter duration for higher CPS
            state->cpsModifier = config->spikeCPS;
            int minDuration = baseCPS > 15 ? 1 : 2;
            int maxDuration = baseCPS > 15 ? 3 : 4;
            state->remainingTicks = (int)enhanced_randomization(minDuration, maxDuration + 1, state);
            finalCPS += state->cpsModifier;
        }
    }
    
    // Ensure minimum CPS and reasonable maximum
    if (finalCPS < 0.5f) finalCPS = 0.5f;
    if (finalCPS > baseCPS * 2.0f) finalCPS = baseCPS * 2.0f;
    
    state->lastCpsValue = finalCPS;
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