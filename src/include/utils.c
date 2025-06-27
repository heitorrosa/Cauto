#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <math.h> 

#include "utils.h"

void init_config(configCauto *config) {
    config->leftActive = true;
    config->mcOnly = true;
    config->clickInventory = false;
    config->breakBlocks = true;
    config->soundClicks = true;

    config->inputCPS = 13;
    config->minDurationClick = 22;
    config->maxDurationClick = 30;

    config->dropChance = 50;
    config->spikeChance = 50;
    config->dropCPS = 3;
    config->spikeCPS = 2;
}

void init_randomState(RandomState *state) {
    state->remainingTicks = 0;
    state->cpsModifier = 0;
    
    // Initialize multiple entropy sources
    DWORD tick = GetTickCount();
    DWORD pid = GetCurrentProcessId();
    state->linearSeed = tick ^ pid;
    state->xorshiftSeed = (tick << 16) | (pid & 0xFFFF);
    state->lfsr = 0xACE1u; // Non-zero seed for LFSR
    
    // Initialize timing system
    QueryPerformanceFrequency(&state->frequency);
    QueryPerformanceCounter(&state->lastClickTime);
    
    // Initialize history tracking
    for (int i = 0; i < 32; i++) {
        state->clickHistory[i] = 0.0f;
    }
    state->historyIndex = 0;
    state->fatigueLevel = 0.0f;
    state->consistentClickCount = 0;
    state->sessionStartTime = GetTickCount();
}

// High-quality random using multiple algorithms
float randomization(float min, float max, RandomState *state) {
    // Linear Congruential Generator
    state->linearSeed = (1103515245U * state->linearSeed + 12345U) & 0x7fffffffU;
    float rand1 = (float)state->linearSeed / 0x7fffffffU;
    
    // XorShift32
    state->xorshiftSeed ^= state->xorshiftSeed << 13;
    state->xorshiftSeed ^= state->xorshiftSeed >> 17;
    state->xorshiftSeed ^= state->xorshiftSeed << 5;
    float rand2 = (float)(state->xorshiftSeed & 0x7fffffffU) / 0x7fffffffU;
    
    // Linear Feedback Shift Register
    unsigned int lsb = state->lfsr & 1;
    state->lfsr >>= 1;
    if (lsb) state->lfsr ^= 0xB400u;
    float rand3 = (float)state->lfsr / 0xFFFFU;
    
    // Combine all three with weights
    float combined = (rand1 * 0.5f + rand2 * 0.3f + rand3 * 0.2f);
    
    return min + combined * (max - min);
}

// Gaussian (normal) distribution using Box-Muller transform
float gaussianRandom(GaussianState *gauss, RandomState *state) {
    if (gauss->hasSpare) {
        gauss->hasSpare = false;
        return gauss->spare * gauss->stddev + gauss->mean;
    }
    
    gauss->hasSpare = true;
    float u = randomization(0.0001f, 1.0f, state); // Avoid exact 0
    float v = randomization(0.0001f, 1.0f, state);
    
    float mag = gauss->stddev * sqrtf(-2.0f * logf(u));
    gauss->spare = mag * cosf(6.28318530718f * v);
    
    return mag * sinf(6.28318530718f * v) + gauss->mean;
}

// Adaptive randomization that learns from click patterns
float adaptiveRandomization(float base, float variance, RandomState *state) {
    // Calculate current time
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    // Calculate interval since last click
    float interval = (float)(currentTime.QuadPart - state->lastClickTime.QuadPart) / state->frequency.QuadPart * 1000.0f;
    
    // Store in history
    state->clickHistory[state->historyIndex] = interval;
    state->historyIndex = (state->historyIndex + 1) % 32;
    
    // Calculate variance in recent clicks
    float sum = 0, sumSq = 0;
    for (int i = 0; i < 32; i++) {
        if (state->clickHistory[i] > 0) {
            sum += state->clickHistory[i];
            sumSq += state->clickHistory[i] * state->clickHistory[i];
        }
    }
    float mean = sum / 32.0f;
    float stddev = sqrtf(sumSq / 32.0f - mean * mean);
    
    // Increase variance if we're being too consistent
    if (stddev < variance * 0.3f) {
        variance *= 1.5f;
        state->consistentClickCount++;
    } else {
        state->consistentClickCount = 0;
    }
    
    // Add fatigue over time (makes clicks slightly less consistent)
    DWORD sessionTime = GetTickCount() - state->sessionStartTime;
    state->fatigueLevel = (float)(sessionTime / 60000) * 0.1f; // 0.1 per minute
    if (state->fatigueLevel > 1.0f) state->fatigueLevel = 1.0f;
    
    variance += variance * state->fatigueLevel;
    
    // Use Gaussian distribution for more natural timing
    GaussianState gauss = {base, variance * 0.3f, false, 0.0f};
    float result = gaussianRandom(&gauss, state);
    
    // Update last click time
    state->lastClickTime = currentTime;
    
    return result;
}

// Calculate micro-jitter for click timing
float calculateJitter(RandomState *state) {
    // Create small, natural variations (±2ms typical)
    float jitter = randomization(-2.0f, 2.0f, state);
    
    // Add occasional larger variations (human inconsistency)
    if (randomization(0, 100, state) < 5) { // 5% chance
        jitter += randomization(-5.0f, 5.0f, state);
    }
    
    return jitter;
}

// Enhanced CPS calculation with better human simulation
float cpsWithBursts(configCauto *config, RandomState *state) {
    float baseCPS = (float)config->inputCPS;
    float finalCPS = baseCPS;
    
    // Continue existing burst/drop
    if (state->remainingTicks > 0) {
        finalCPS += state->cpsModifier;
        state->remainingTicks--;
        
        // Natural decay of burst intensity
        if (state->remainingTicks > 0) {
            float decayFactor = 0.85f + randomization(0.0f, 0.1f, state);
            state->cpsModifier = (int)(state->cpsModifier * decayFactor);
        }
    } else {
        // Determine if new burst/drop should occur
        float baseChance = randomization(0, 100, state);
        
        // Adjust chances based on session time and fatigue
        DWORD sessionTime = GetTickCount() - state->sessionStartTime;
        float timeMultiplier = 1.0f + (sessionTime / 300000.0f) * 0.2f; // Increase over 5 minutes
        
        float adjustedDropChance = config->dropChance * timeMultiplier;
        float adjustedSpikeChance = config->spikeChance * (2.0f - timeMultiplier * 0.5f);
        
        if (baseChance < adjustedDropChance) {
            // Start drop sequence with variable intensity
            int dropIntensity = (int)randomization(1, config->dropCPS + 1, state);
            state->cpsModifier = -dropIntensity;
            state->remainingTicks = (int)randomization(2, 6, state);
            finalCPS += state->cpsModifier;
        } else if (baseChance < adjustedDropChance + adjustedSpikeChance) {
            // Start spike sequence
            int spikeIntensity = (int)randomization(1, config->spikeCPS + 1, state);
            state->cpsModifier = spikeIntensity;
            state->remainingTicks = (int)randomization(2, 4, state);
            finalCPS += state->cpsModifier;
        } else {
            // Natural micro-variations even without bursts
            float microVariation = randomization(-0.8f, 0.8f, state);
            
            // Add slight tremor simulation
            if (randomization(0, 100, state) < 15) { // 15% chance
                microVariation += randomization(-0.3f, 0.3f, state);
            }
            
            finalCPS += microVariation;
        }
    }
    
    // Ensure reasonable bounds
    if (finalCPS < 1.0f) finalCPS = 1.0f;
    if (finalCPS > baseCPS * 2.5f) finalCPS = baseCPS * 2.5f; // Prevent extreme spikes
    
    return finalCPS;
}

bool cursorVisible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;
    
    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void sendLeftClickDown(bool down) {
    POINT pos;
    GetCursorPos(&pos);

    PostMessageA(GetForegroundWindow(), down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}