#include "../resources/include.c"

#include "clicker.h"
#include "../utils/utils.h"

void initClickerConfig (clickerConfig *clicker) {
    clicker->inputCPS = 13;
    clicker->minDurationClick = 22;
    clicker->maxDurationClick = 30;

    clicker->dropChance = 50;
    clicker->spikeChance = 50;
    clicker->dropCPS = 3;
    clicker->spikeCPS = 2;
}

void initRandomState(RandomState *state) {
    // Initialize high-quality entropy
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    DWORD tick = GetTickCount();
    DWORD pid = GetCurrentProcessId();
    
    state->entropy = (unsigned long long)perfCounter.QuadPart ^ 
                    ((unsigned long long)tick << 32) ^ 
                    ((unsigned long long)pid << 16);
    
    // Initialize ChaCha20-like state for cryptographic quality randomness
    state->chacha_state[0] = 0x61707865; state->chacha_state[1] = 0x3320646e;
    state->chacha_state[2] = 0x79622d32; state->chacha_state[3] = 0x6b206574;
    state->chacha_state[4] = (unsigned int)(state->entropy & 0xFFFFFFFF);
    state->chacha_state[5] = (unsigned int)(state->entropy >> 32);
    state->chacha_state[6] = tick; state->chacha_state[7] = pid;
    for (int i = 8; i < 16; i++) {
        state->chacha_state[i] = tick ^ (pid << (i - 8));
    }
    state->counter = 0;
    
    // Initialize human behavioral state
    state->human.concentration = 0.85f + (float)(state->entropy % 100) / 1000.0f; // 0.85-0.95
    state->human.fatigue = 0.0f;
    state->human.rhythm = 1.0f;
    state->human.microPauseCounter = 0;
    
    // Initialize anti-detection enhancements
    state->human.exhaustionLevel = 0.0f;
    state->human.outlierForced = 0;
    state->human.kurtosisCounter = 0.0f;
    state->human.needsBreak = false;
    state->human.lastBreakTime = GetTickCount();
    
    // Initialize recovery system
    state->human.lastActiveTime = GetTickCount();
    state->human.isRecovering = false;
    state->human.recoveryRate = 1.0f + (float)(state->entropy % 50) / 100.0f; // 1.0-1.5 recovery multiplier
    state->human.wasInactive = false;
    
    // Initialize timing system
    QueryPerformanceFrequency(&state->frequency);
    QueryPerformanceCounter(&state->lastClickTime);
    
    // Initialize other state
    state->remainingTicks = 0;
    state->cpsModifier = 0;
    for (int i = 0; i < 16; i++) state->recentIntervals[i] = 0.0f;
    state->intervalIndex = 0;
    state->baseVariation = 0.05f + (float)(state->entropy % 50) / 2000.0f; // 0.05-0.075
    state->momentaryBias = 0.0f;
    state->sessionStartTime = GetTickCount();
    state->inCluster = false;
    state->clusterRemaining = 0;
    state->clusterSpeedModifier = 1.0f;
}

// High-quality random number generator using ChaCha20-like algorithm
static unsigned int advancedRandom(RandomState *state) {
    // Simplified ChaCha20 quarter-round
    unsigned int *s = state->chacha_state;
    
    // Add counter to maintain stream cipher properties
    s[12] = state->counter++;
    
    // ChaCha20 quarter-round operations (simplified)
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 16) | (s[12] >> 16);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 12) | (s[4] >> 20);
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 8) | (s[12] >> 24);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 7) | (s[4] >> 25);
    
    return s[0] ^ s[4] ^ s[8] ^ s[12];
}

static float randomFloat(RandomState *state) {
    return (float)(advancedRandom(state) & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

static float randomRange(float min, float max, RandomState *state) {
    return min + randomFloat(state) * (max - min);
}

// Box-Muller transform for Gaussian distribution
static float gaussianRandom(float mean, float stddev, RandomState *state) {
    static bool hasSpare = false;
    static float spare;
    
    if (hasSpare) {
        hasSpare = false;
        return spare * stddev + mean;
    }
    
    hasSpare = true;
    float u = randomFloat(state);
    float v = randomFloat(state);
    
    // Prevent log(0)
    if (u < 1e-7f) u = 1e-7f;
    
    float mag = stddev * sqrtf(-2.0f * logf(u));
    spare = mag * cosf(6.28318530718f * v);
    
    return mag * sinf(6.28318530718f * v) + mean;
}

// Update human behavioral state with gradual fatigue recovery
static void updateHumanState(RandomState *state, bool isActivelyClicking) {
    DWORD currentTime = GetTickCount();
    DWORD sessionDuration = currentTime - state->sessionStartTime;
    DWORD timeSinceBreak = currentTime - state->human.lastBreakTime;
    DWORD timeSinceActive = currentTime - state->human.lastActiveTime;
    
    if (isActivelyClicking) {
        // Check if user was previously inactive (break of 2+ seconds)
        if (state->human.wasInactive && timeSinceActive > 2000) {
            // User is resuming clicking after a break - gradual fatigue recovery
            float breakDuration = (float)timeSinceActive / 1000.0f; // seconds
            
            if (breakDuration >= 2.0f) {
                // Progressive fatigue reduction based on break duration
                float fatigueReduction;
                
                if (breakDuration <= 5.0f) {
                    // 2-5 seconds: 30-50% reduction
                    fatigueReduction = 0.3f + ((breakDuration - 2.0f) / 3.0f) * 0.2f;
                } else if (breakDuration <= 15.0f) {
                    // 5-15 seconds: 50-65% reduction
                    fatigueReduction = 0.5f + ((breakDuration - 5.0f) / 10.0f) * 0.15f;
                } else if (breakDuration <= 30.0f) {
                    // 15-30 seconds: 65-75% reduction
                    fatigueReduction = 0.65f + ((breakDuration - 15.0f) / 15.0f) * 0.1f;
                } else {
                    // 30+ seconds: 75-80% reduction (cap)
                    fatigueReduction = 0.75f + (fminf(breakDuration - 30.0f, 30.0f) / 30.0f) * 0.05f;
                }
                
                // Apply gradual recovery
                state->human.fatigue *= (1.0f - fatigueReduction);
                state->human.exhaustionLevel *= (1.0f - fatigueReduction);
                
                // More conservative session timer reset to avoid detection spikes
                if (breakDuration > 15.0f && state->human.fatigue < 0.15f) {
                    // Only partial session reset to maintain some behavioral continuity
                    DWORD resetAmount = (DWORD)(fatigueReduction * 180000.0f); // Max 3 minutes back
                    state->sessionStartTime = currentTime - resetAmount;
                }
            }
        }
        
        state->human.lastActiveTime = currentTime;
        state->human.isRecovering = false;
        state->human.wasInactive = false;
        
        // Normal fatigue accumulation when actively clicking
        float timeFactor = (float)sessionDuration / 60000.0f;
        state->human.fatigue = 0.3f * (1.0f - expf(-timeFactor * 0.08f));
        
        if (timeFactor > 8.0f) {
            float exhaustionTime = timeFactor - 8.0f;
            state->human.exhaustionLevel = 0.15f * (1.0f - expf(-exhaustionTime * 0.1f));
        }
    } else {
        // Mark as inactive if break is long enough
        if (timeSinceActive > 2000) {
            state->human.wasInactive = true;
            state->human.isRecovering = true;
            
            // Much slower gradual fatigue recovery during inactivity
            float recoveryTime = (float)timeSinceActive / 1000.0f;
            float fatigueRecovery = state->human.recoveryRate * recoveryTime * 0.01f; // Reduced from 0.02f to 0.01f
            
            // Less aggressive accelerated recovery
            if (recoveryTime <= 30.0f) {
                fatigueRecovery *= 1.5f; // Reduced from 2.0f to 1.5f
            }
            
            state->human.fatigue -= fatigueRecovery;
            if (state->human.fatigue < 0.0f) state->human.fatigue = 0.0f;
            
            float exhaustionRecovery = state->human.recoveryRate * recoveryTime * 0.005f; // Reduced from 0.01f
            state->human.exhaustionLevel -= exhaustionRecovery;
            if (state->human.exhaustionLevel < 0.0f) state->human.exhaustionLevel = 0.0f;
            
            // More conservative session timer reset
            if (recoveryTime > 120.0f && state->human.fatigue < 0.05f) { // Increased threshold
                state->sessionStartTime = currentTime - 60000;
            }
        }
    }
    
    // Concentration calculation with smoother transitions
    float concentrationBase = 0.90f - (state->human.fatigue * 0.2f) - (state->human.exhaustionLevel * 0.3f);
    
    // Reduce concentration bonus during early recovery to prevent detection spikes
    if (state->human.isRecovering) {
        float recoveryTime = (float)(currentTime - state->human.lastActiveTime) / 1000.0f;
        if (recoveryTime < 10.0f) {
            // Gradually increase concentration bonus over first 10 seconds of recovery
            concentrationBase += 0.05f * (recoveryTime / 10.0f);
        } else {
            concentrationBase += 0.05f; // Reduced from 0.1f
        }
    }
    
    float microCycle = sinf((float)sessionDuration / 25000.0f * 6.28318530718f) * 0.1f;
    state->human.concentration = concentrationBase + microCycle;
    if (state->human.concentration > 1.0f) state->human.concentration = 1.0f;
    if (state->human.concentration < 0.6f) state->human.concentration = 0.6f;
    
    // Smoother rhythm changes to prevent detection spikes
    float rhythmVariation = 0.015f + (state->human.fatigue * 0.02f) + (state->human.exhaustionLevel * 0.03f);
    
    // Reduce rhythm variation during early recovery
    if (state->human.isRecovering) {
        float recoveryTime = (float)(currentTime - state->human.lastActiveTime) / 1000.0f;
        if (recoveryTime < 15.0f) {
            rhythmVariation *= 0.3f + (recoveryTime / 15.0f) * 0.4f; // Gradual increase from 30% to 70%
        } else {
            rhythmVariation *= 0.7f; // Reduced from 0.5f
        }
    }
    
    state->human.rhythm += randomRange(-rhythmVariation, rhythmVariation, state);
    if (state->human.rhythm < 0.7f) state->human.rhythm = 0.7f;
    if (state->human.rhythm > 1.25f) state->human.rhythm = 1.25f;
    
    // Extremely conservative kurtosis manipulation - barely noticeable
    if (isActivelyClicking) {
        state->human.kurtosisCounter += 0.3f; // Further reduced from 0.5f
    }
    
    // Extremely rare outlier forcing to prevent consistency detection
    if (isActivelyClicking) {
        state->human.outlierForced++;
        
        // Make outliers extremely rare
        float outlierThreshold = 40.0f + randomRange(0.0f, 60.0f, state); // Increased from 20-50 to 40-100
        if (state->human.wasInactive) {
            float timeSinceRecovery = (float)(currentTime - state->human.lastActiveTime) / 1000.0f;
            if (timeSinceRecovery < 60.0f) { // Extended from 45s to 60s
                // Make outliers even rarer during recovery
                outlierThreshold *= 3.0f + (60.0f - timeSinceRecovery) / 30.0f; // 3x to 5x threshold
            }
        }
        
        if (state->human.outlierForced > (int)outlierThreshold) {
            // Even less chance to force outliers
            if (randomFloat(state) < 0.5f) { // Reduced from 70% to 50%
                state->human.outlierForced = -(1 + (int)randomRange(0.0f, 0.5f, state)); // Max 1-2 outliers
            } else {
                state->human.outlierForced = 0; // Skip outliers more often
            }
        }
    }
    
    // Moderate micro-pause system with smoother recovery
    if (isActivelyClicking) {
        state->human.microPauseCounter++;
        int pauseThreshold = (int)(120.0f - (state->human.exhaustionLevel * 30.0f));
        
        // Increase pause threshold during early recovery
        if (state->human.wasInactive) {
            float timeSinceRecovery = (float)(currentTime - state->human.lastActiveTime) / 1000.0f;
            if (timeSinceRecovery < 20.0f) {
                pauseThreshold = (int)(pauseThreshold * (1.3f + (20.0f - timeSinceRecovery) / 40.0f));
            }
        }
        
        if (state->human.microPauseCounter > pauseThreshold) {
            float pauseChance = 0.1f + (state->human.fatigue * 0.15f) + (state->human.exhaustionLevel * 0.2f);
            if (randomFloat(state) < pauseChance) {
                int pauseDuration = (int)(5.0f + (state->human.exhaustionLevel * 15.0f));
                state->human.microPauseCounter = -pauseDuration;
            } else {
                state->human.microPauseCounter = 0;
            }
        }
    }
}

// Calculate CPS with proper bounds
static float calculateHumanCPS(clickerConfig *clicker, RandomState *state, bool isActivelyClicking) {
    updateHumanState(state, isActivelyClicking);
    
    float baseCPS = (float)clicker->inputCPS;
    float modifiedCPS = baseCPS;
    
    // Moderate behavioral modifiers
    modifiedCPS *= state->human.rhythm;
    modifiedCPS *= (0.75f + state->human.concentration * 0.25f);
    
    // Reduced exhaustion impact
    if (state->human.exhaustionLevel > 0.05f) {
        float exhaustionMultiplier = 1.0f - (state->human.exhaustionLevel * 0.4f);
        modifiedCPS *= exhaustionMultiplier;
        
        if (randomFloat(state) < state->human.exhaustionLevel * 0.05f) {
            modifiedCPS *= 0.6f;
        }
    }
    
    // Shorter, less severe breaks
    if (state->human.needsBreak) {
        modifiedCPS *= 0.4f;
        if (randomFloat(state) < 0.5f) {
            state->human.needsBreak = false;
            state->human.lastBreakTime = GetTickCount();
        }
        return modifiedCPS;
    }
    
    // Moderate micro-pauses
    if (state->human.microPauseCounter < 0) {
        float pauseMultiplier = 0.3f;
        modifiedCPS *= pauseMultiplier;
        state->human.microPauseCounter++;
        return modifiedCPS;
    }
    
    // Extremely conservative outlier forcing - barely noticeable
    if (state->human.outlierForced < 0) {
        float outlierMultiplier;
        // Only use very mild outliers
        if (randomFloat(state) < 0.5f) {
            outlierMultiplier = randomRange(0.85f, 0.95f, state); // Very mild slow
        } else {
            outlierMultiplier = randomRange(1.05f, 1.15f, state); // Very mild fast
        }
        
        modifiedCPS *= outlierMultiplier;
        state->human.outlierForced++;
        
        // Skip other modifications for clean outliers
        goto bounds_check;
    }
    
    // Enhanced temporal clustering
    if (!state->inCluster && randomFloat(state) < (0.12f + state->human.fatigue * 0.03f)) {
        state->inCluster = true;
        state->clusterRemaining = (int)randomRange(3.0f, 7.0f, state);
        state->clusterSpeedModifier = randomRange(0.8f, 1.3f, state);
    }
    
    if (state->inCluster) {
        modifiedCPS *= state->clusterSpeedModifier;
        state->clusterRemaining--;
        if (state->clusterRemaining <= 0) {
            state->inCluster = false;
        }
    }
    
    // Standard burst/drop system with fatigue influence
    if (state->remainingTicks > 0) {
        float concentrationFactor = 0.5f + state->human.concentration * 0.5f;
        modifiedCPS += state->cpsModifier * concentrationFactor;
        state->remainingTicks--;
    } else {
        float randomVal = randomFloat(state) * 100.0f;
        float fatigueMultiplier = 1.0f + (state->human.fatigue * 0.8f) + (state->human.exhaustionLevel * 0.6f);
        
        if (randomVal < clicker->dropChance * fatigueMultiplier) {
            int dropIntensity = (int)randomRange(1.0f, (float)clicker->dropCPS + 1.0f, state);
            if (state->human.exhaustionLevel > 0.1f) {
                dropIntensity += (int)(state->human.exhaustionLevel * 20.0f);
            }
            state->cpsModifier = -dropIntensity;
            state->remainingTicks = (int)randomRange(2.0f, 5.0f, state);
            modifiedCPS += state->cpsModifier;
        } else if (randomVal < (clicker->dropChance + clicker->spikeChance) * (2.2f - fatigueMultiplier * 0.6f)) {
            int spikeIntensity = (int)randomRange(1.0f, (float)clicker->spikeCPS + 1.0f, state);
            spikeIntensity = (int)(spikeIntensity * (1.0f - state->human.exhaustionLevel * 0.3f));
            state->cpsModifier = spikeIntensity;
            state->remainingTicks = (int)randomRange(2.0f, 4.0f, state);
            modifiedCPS += state->cpsModifier;
        }
    }
    
    // Extremely conservative kurtosis manipulation - barely detectable
    float kurtosisPhase = fmodf(state->human.kurtosisCounter, 120.0f); // Even longer cycle
    if (kurtosisPhase < 60.0f) {
        // High kurtosis period - normal Gaussian behavior
        float kurtosisVariation = gaussianRandom(0.0f, baseCPS * 0.015f, state); // Very small variation
        modifiedCPS += kurtosisVariation;
    } else {
        // Low kurtosis period - extremely conservative
        float kurtosisVariation = randomRange(-baseCPS * 0.04f, baseCPS * 0.04f, state); // Very small range
        // Heavy Gaussian component to maintain normal-ish distribution
        kurtosisVariation += gaussianRandom(0.0f, baseCPS * 0.06f, state); // Dominant Gaussian
        modifiedCPS += kurtosisVariation;
    }
    
    // Minimal natural micro-variations
    float microVariation = gaussianRandom(0.0f, baseCPS * 0.02f, state); // Further reduced
    modifiedCPS += microVariation;
    
    // Very conservative momentary bias
    state->momentaryBias += randomRange(-0.03f, 0.03f, state); // Further reduced
    state->momentaryBias *= 0.99f; // Even faster decay
    modifiedCPS += state->momentaryBias;

bounds_check:
    // Tighter bounds - should not exceed inputCPS + spikeCPS
    float minCPS = baseCPS * 0.7f; // 70% minimum
    float maxCPS = baseCPS + (float)clicker->spikeCPS; // inputCPS + spikeCPS maximum
    
    if (modifiedCPS < minCPS) modifiedCPS = minCPS;
    if (modifiedCPS > maxCPS) modifiedCPS = maxCPS;
    
    return modifiedCPS;
}

// Main click interval calculation with activity detection
float getClickInterval(clickerConfig *clicker, RandomState *state) {
    // Determine if actively clicking based on recent call frequency
    DWORD currentTimeMs = GetTickCount();
    bool isActivelyClicking = (currentTimeMs - state->human.lastActiveTime) < 1000;
    
    float cps = calculateHumanCPS(clicker, state, isActivelyClicking);
    float baseInterval = 1000.0f / cps;
    
    // Calculate current time for adaptive timing
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    if (state->lastClickTime.QuadPart != 0) {
        float actualInterval = (float)(currentTime.QuadPart - state->lastClickTime.QuadPart) / state->frequency.QuadPart * 1000.0f;
        
        // Store in recent intervals
        state->recentIntervals[state->intervalIndex] = actualInterval;
        state->intervalIndex = (state->intervalIndex + 1) % 16;
        
        // Extremely conservative consistency analysis - almost never trigger
        float sum = 0, count = 0;
        for (int i = 0; i < 16; i++) {
            if (state->recentIntervals[i] > 0) {
                sum += state->recentIntervals[i];
                count++;
            }
        }
        
        if (count > 14) { // Only analyze when buffer is almost full
            float recentMean = sum / count;
            float variance = 0;
            for (int i = 0; i < 16; i++) {
                if (state->recentIntervals[i] > 0) {
                    float diff = state->recentIntervals[i] - recentMean;
                    variance += diff * diff;
                }
            }
            variance /= count;
            float stddev = sqrtf(variance);
            
            // Extremely high threshold - only intervene if impossibly consistent
            float consistencyRatio = stddev / recentMean;
            if (consistencyRatio < 0.03f) { // Further reduced from 0.05f
                baseInterval += randomRange(-baseInterval * 0.05f, baseInterval * 0.05f, state); // Minimal variation
            }
        }
    }
    
    state->lastClickTime = currentTime;
    
    // Very conservative human inconsistency
    float humanVariation = state->baseVariation * baseInterval * (1.0f + state->human.fatigue * 0.2f); // Further reduced
    
    // Minimal kurtosis-based variation
    float kurtosisPhase = fmodf(state->human.kurtosisCounter, 120.0f);
    if (kurtosisPhase >= 60.0f) {
        humanVariation *= 1.1f; // Minimal increase from 1.3f
    }
    
    float finalInterval = gaussianRandom(baseInterval, humanVariation, state);
    
    // Ensure minimum interval for usability
    if (finalInterval < 8.0f) finalInterval = 8.0f;
    
    return finalInterval;
}

// Human-like click duration with anti-detection
float getClickDuration(clickerConfig *clicker, RandomState *state) {
    float baseDuration = (clicker->minDurationClick + clicker->maxDurationClick) / 2.0f;
    float range = clicker->maxDurationClick - clicker->minDurationClick;
    
    // Moderate human behavioral influence
    float concentrationFactor = 0.85f + state->human.concentration * 0.3f; // 0.85-1.15
    float fatigueFactor = 1.0f + state->human.fatigue * 0.15f + state->human.exhaustionLevel * 0.1f; // 1.0-1.25
    
    baseDuration *= concentrationFactor * fatigueFactor;
    
    // Natural variation with Gaussian distribution
    float variation = range * 0.25f;
    float duration = gaussianRandom(baseDuration, variation, state);
    
    // Ensure bounds
    if (duration < clicker->minDurationClick) duration = clicker->minDurationClick;
    if (duration > clicker->maxDurationClick) duration = clicker->maxDurationClick;
    
    return duration;
}