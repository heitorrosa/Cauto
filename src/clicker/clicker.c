#include "../common/common.h"

#include "clicker.h"
#include "../utils/utils.h"

void initClickerConfig (clickerConfig *clicker) {
    clicker->inputCPS = 13;
    
    clicker->minDurationClick = 22;
    clicker->maxDurationClick = 30;

    clicker->dropChance = 50;
    clicker->dropCPS = 3;

    clicker->spikeChance = 50;
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
    spare = mag * cosf(TWO_PI * v);
    
    return mag * sinf(TWO_PI * v) + mean;
}

// Natural human behavioral state update
static void updateHumanState(RandomState *state, bool isActivelyClicking) {
    DWORD currentTime = GetTickCount();
    DWORD sessionDuration = currentTime - state->sessionStartTime;
    DWORD timeSinceActive = currentTime - state->human.lastActiveTime;
    
    if (isActivelyClicking) {
        // Fatigue recovery on resume
        if (state->human.wasInactive && timeSinceActive > 2000) {
            float breakDuration = (float)timeSinceActive / 1000.0f;
            float fatigueReduction = fminf(0.8f, 0.3f + (breakDuration / 60.0f));
            
            state->human.fatigue *= (1.0f - fatigueReduction);
            state->human.exhaustionLevel *= (1.0f - fatigueReduction);
            
            if (breakDuration > 15.0f) {
                state->sessionStartTime = currentTime - (DWORD)(fatigueReduction * 120000.0f);
            }
        }
        
        state->human.lastActiveTime = currentTime;
        state->human.isRecovering = false;
        state->human.wasInactive = false;
        
        // Minimal fatigue accumulation
        float timeFactor = (float)sessionDuration / 600000.0f; // 10 minutes to reach max
        state->human.fatigue = 0.1f * (1.0f - expf(-timeFactor * 0.3f));
        
        if (timeFactor > 3.0f) { // After 30 minutes
            float exhaustionTime = timeFactor - 3.0f;
            state->human.exhaustionLevel = 0.03f * (1.0f - expf(-exhaustionTime * 0.2f));
        }
    } else {
        if (timeSinceActive > 2000) {
            state->human.wasInactive = true;
            state->human.isRecovering = true;
            
            float recoveryTime = (float)timeSinceActive / 1000.0f;
            float recovery = recoveryTime * 0.025f;
            
            state->human.fatigue -= recovery;
            if (state->human.fatigue < 0.0f) state->human.fatigue = 0.0f;
            
            state->human.exhaustionLevel -= recovery * 0.8f;
            if (state->human.exhaustionLevel < 0.0f) state->human.exhaustionLevel = 0.0f;
        }
    }
    
    // Natural concentration with realistic variation
    float baseConcentration = 0.85f - (state->human.fatigue * 0.2f) - (state->human.exhaustionLevel * 0.15f);
    
    // Add natural attention fluctuations
    float attentionWave = sinf((float)sessionDuration / 20000.0f * TWO_PI) * 0.06f;
    float microFocus = randomRange(-0.04f, 0.04f, state);
    
    state->human.concentration = baseConcentration + attentionWave + microFocus;
    if (state->human.concentration > 1.0f) state->human.concentration = 1.0f;
    if (state->human.concentration < 0.65f) state->human.concentration = 0.65f;
    
    // Natural rhythm with realistic drift
    float rhythmDrift = randomRange(-0.01f, 0.01f, state);
    float fatigueRhythm = state->human.fatigue * randomRange(-0.02f, 0.02f, state);
    
    state->human.rhythm += rhythmDrift + fatigueRhythm;
    if (state->human.rhythm < 0.88f) state->human.rhythm = 0.88f;
    if (state->human.rhythm > 1.12f) state->human.rhythm = 1.12f;
    
    // Natural counters
    if (isActivelyClicking) {
        state->human.outlierForced++;
        state->human.microPauseCounter++;
        
        // Realistic micro-pauses based on concentration
        int pauseThreshold = (int)(200.0f + randomRange(-50.0f, 50.0f, state));
        if (state->human.microPauseCounter > pauseThreshold) {
            float pauseChance = 0.03f + (state->human.fatigue * 0.05f);
            if (randomFloat(state) < pauseChance) {
                state->human.microPauseCounter = -(int)randomRange(1.0f, 3.0f, state);
            } else {
                state->human.microPauseCounter = 0;
            }
        }
    }
}

// Natural CPS calculation with proper variance
static float calculateHumanCPS(clickerConfig *clicker, RandomState *state, bool isActivelyClicking) {
    updateHumanState(state, isActivelyClicking);
    
    float baseCPS = (float)clicker->inputCPS;
    float modifiedCPS = baseCPS;
    
    // Natural human behavioral variation
    modifiedCPS *= state->human.rhythm;
    modifiedCPS *= (0.85f + state->human.concentration * 0.15f);
    
    // Light fatigue effects
    if (state->human.exhaustionLevel > 0.01f) {
        modifiedCPS *= (1.0f - state->human.exhaustionLevel * 0.4f);
    }
    
    // Natural micro-pauses
    if (state->human.microPauseCounter < 0) {
        modifiedCPS *= 0.6f;
        state->human.microPauseCounter++;
        return modifiedCPS;
    }
    
    // Natural outliers with proper frequency - every 15-30 clicks
    if (state->human.outlierForced > (15 + (int)randomRange(0.0f, 15.0f, state))) {
        float outlierType = randomFloat(state);
        float outlierStrength = randomFloat(state);
        
        if (outlierType < 0.35f) {
            // Slow outliers (35%)
            if (outlierStrength < 0.3f) {
                modifiedCPS *= randomRange(0.3f, 0.6f, state); // Strong slow
            } else {
                modifiedCPS *= randomRange(0.6f, 0.8f, state); // Mild slow
            }
        } else if (outlierType < 0.7f) {
            // Fast outliers (35%)
            if (outlierStrength < 0.3f) {
                modifiedCPS *= randomRange(1.6f, 2.0f, state); // Strong fast
            } else {
                modifiedCPS *= randomRange(1.2f, 1.6f, state); // Mild fast
            }
        } else {
            // Moderate variations (30%)
            modifiedCPS *= randomRange(0.85f, 1.15f, state);
        }
        
        state->human.outlierForced = 0;
        goto bounds_check;
    }
    
    // Natural clustering with realistic patterns
    if (!state->inCluster && randomFloat(state) < (0.06f + state->human.fatigue * 0.02f)) {
        state->inCluster = true;
        state->clusterRemaining = (int)randomRange(2.0f, 6.0f, state);
        // More varied cluster speeds
        float clusterRoll = randomFloat(state);
        if (clusterRoll < 0.3f) {
            state->clusterSpeedModifier = randomRange(0.8f, 0.9f, state); // Slow cluster
        } else if (clusterRoll < 0.6f) {
            state->clusterSpeedModifier = randomRange(1.1f, 1.2f, state); // Fast cluster
        } else {
            state->clusterSpeedModifier = randomRange(0.95f, 1.05f, state); // Normal cluster
        }
    }
    
    if (state->inCluster) {
        modifiedCPS *= state->clusterSpeedModifier;
        state->clusterRemaining--;
        if (state->clusterRemaining <= 0) {
            state->inCluster = false;
        }
    }
    
    // Enhanced burst/drop system
    if (state->remainingTicks > 0) {
        float concentrationFactor = 0.7f + state->human.concentration * 0.3f;
        float proposedCPS = modifiedCPS + (state->cpsModifier * concentrationFactor);
        
        if (state->cpsModifier > 0) {
            float maxCPS = baseCPS + (float)clicker->spikeCPS;
            if (proposedCPS > maxCPS) {
                state->cpsModifier = (int)(maxCPS - modifiedCPS);
            }
        }
        modifiedCPS += state->cpsModifier * concentrationFactor;
        state->remainingTicks--;
    } else {
        float randomVal = randomFloat(state) * 100.0f;
        float fatigueInfluence = 1.0f + (state->human.fatigue * 0.3f);
        
        if (randomVal < clicker->dropChance * fatigueInfluence) {
            state->cpsModifier = -(int)randomRange(1.0f, (float)clicker->dropCPS + 1.0f, state);
            state->remainingTicks = (int)randomRange(1.0f, 3.0f, state);
            modifiedCPS += state->cpsModifier;
        } else if (randomVal < (clicker->dropChance + clicker->spikeChance) * (1.8f - fatigueInfluence * 0.2f)) {
            int spike = (int)randomRange(1.0f, (float)clicker->spikeCPS + 1.0f, state);
            float maxCPS = baseCPS + (float)clicker->spikeCPS;
            if (modifiedCPS + spike > maxCPS) {
                spike = (int)(maxCPS - modifiedCPS);
                if (spike < 1) spike = 1;
            }
            state->cpsModifier = spike;
            state->remainingTicks = (int)randomRange(1.0f, 3.0f, state);
            modifiedCPS += state->cpsModifier;
        }
    }
    
    // Multi-layered natural variation for proper variance
    float primaryVariation = gaussianRandom(0.0f, baseCPS * 0.015f, state);
    float secondaryVariation = randomRange(-baseCPS * 0.012f, baseCPS * 0.012f, state);
    float microVariation = gaussianRandom(0.0f, baseCPS * 0.008f, state);
    
    modifiedCPS += primaryVariation + secondaryVariation + microVariation;
    
    // Natural momentum/bias that creates realistic deviation patterns
    float momentumChange = randomRange(-0.015f, 0.015f, state);
    state->momentaryBias += momentumChange;
    state->momentaryBias *= 0.992f; // Slow decay
    
    // Apply bias with concentration influence
    float biasStrength = 1.0f - (state->human.concentration * 0.3f);
    modifiedCPS += state->momentaryBias * biasStrength;

bounds_check:
    // Natural bounds - not too restrictive
    float minCPS = baseCPS * 0.75f;
    float maxCPS = baseCPS + (float)clicker->spikeCPS;
    
    if (modifiedCPS < minCPS) modifiedCPS = minCPS;
    if (modifiedCPS > maxCPS) modifiedCPS = maxCPS;
    
    return modifiedCPS;
}

// Natural interval calculation with proper variance
float getClickInterval(clickerConfig *clicker, RandomState *state) {
    DWORD currentTimeMs = GetTickCount();
    bool isActivelyClicking = (currentTimeMs - state->human.lastActiveTime) < HUMAN_ACTIVE_WINDOW_MS;
    
    float cps = calculateHumanCPS(clicker, state, isActivelyClicking);
    float baseInterval = 1000.0f / cps;
    
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    if (state->lastClickTime.QuadPart != 0) {
        float actualInterval = (float)(currentTime.QuadPart - state->lastClickTime.QuadPart) / state->frequency.QuadPart * 1000.0f;
        state->recentIntervals[state->intervalIndex] = actualInterval;
        state->intervalIndex = (state->intervalIndex + 1) % 16;
        
        // Natural variance enhancement - if recent intervals are too similar
        float sum = 0, count = 0;
        for (int i = 0; i < 16; i++) {
            if (state->recentIntervals[i] > 0) {
                sum += state->recentIntervals[i];
                count++;
            }
        }
        
        if (count > 8) {
            float recentMean = sum / count;
            float variance = 0;
            for (int i = 0; i < 16; i++) {
                if (state->recentIntervals[i] > 0) {
                    float diff = state->recentIntervals[i] - recentMean;
                    variance += diff * diff;
                }
            }
            variance /= count;
            
            // If variance is too low, add natural human inconsistency
            if (variance < (recentMean * recentMean * 0.004f)) { // Less than 6.3% CV
                float varianceBoost = randomRange(-baseInterval * 0.2f, baseInterval * 0.2f, state);
                baseInterval += varianceBoost;
            }
        }
    }
    
    state->lastClickTime = currentTime;
    
    // Enhanced natural timing variation
    float concentrationEffect = (1.0f - state->human.concentration) * 0.05f;
    float fatigueEffect = state->human.fatigue * 0.03f;
    float baseVariation = state->baseVariation + concentrationEffect + fatigueEffect;
    
    // Multiple sources of natural variation
    float humanVariation = baseVariation * baseInterval;
    float attentionVariation = randomRange(-baseInterval * 0.04f, baseInterval * 0.04f, state);
    float muscleVariation = gaussianRandom(0.0f, baseInterval * 0.02f, state);
    
    float totalVariation = humanVariation + attentionVariation + muscleVariation;
    float finalInterval = gaussianRandom(baseInterval, totalVariation, state);
    
    if (finalInterval < CLICKER_MIN_INTERVAL_MS) finalInterval = CLICKER_MIN_INTERVAL_MS;
    
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