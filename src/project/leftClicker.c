#include "common.h"

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HUMAN_ACTIVE_WINDOW_MS 3000
#define CLICKER_MIN_INTERVAL_MS 5.0f

// ChaCha20-like random number generator for cryptographic quality
static unsigned int advancedRandom() {
    unsigned int *s = randomizer->chacha_state;
    
    // Add counter to maintain stream cipher properties
    s[12] = randomizer->counter++;
    
    // ChaCha20 quarter-round operations
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 16) | (s[12] >> 16);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 12) | (s[4] >> 20);
    s[0] += s[4]; s[12] ^= s[0]; s[12] = (s[12] << 8) | (s[12] >> 24);
    s[8] += s[12]; s[4] ^= s[8]; s[4] = (s[4] << 7) | (s[4] >> 25);
    
    return s[0] ^ s[4] ^ s[8] ^ s[12];
}

static float randomFloat() {
    return (float)(advancedRandom() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

static float randomRange(float min, float max) {
    return min + randomFloat() * (max - min);
}

static float gaussianRandom(float mean, float stddev) {
    static bool hasSpare = false;
    static float spare;
    
    if (hasSpare) {
        hasSpare = false;
        return spare * stddev + mean;
    }
    
    hasSpare = true;
    float u = randomFloat();
    float v = randomFloat();
    
    if (u < 1e-7f) u = 1e-7f;
    
    float mag = stddev * sqrtf(-2.0f * logf(u));
    spare = mag * cosf(TWO_PI * v);
    
    return mag * sinf(TWO_PI * v) + mean;
}

void initRandomizer() {
    // Initialize high-quality entropy
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    DWORD tick = GetTickCount();
    DWORD pid = GetCurrentProcessId();
    
    randomizer->entropy = (unsigned long long)perfCounter.QuadPart ^ 
                         ((unsigned long long)tick << 32) ^ 
                         ((unsigned long long)pid << 16);
    
    // Initialize ChaCha20-like state
    randomizer->chacha_state[0] = 0x61707865; randomizer->chacha_state[1] = 0x3320646e;
    randomizer->chacha_state[2] = 0x79622d32; randomizer->chacha_state[3] = 0x6b206574;
    randomizer->chacha_state[4] = (unsigned int)(randomizer->entropy & 0xFFFFFFFF);
    randomizer->chacha_state[5] = (unsigned int)(randomizer->entropy >> 32);
    randomizer->chacha_state[6] = tick; randomizer->chacha_state[7] = pid;
    for (int i = 8; i < 16; i++) {
        randomizer->chacha_state[i] = tick ^ (pid << (i - 8));
    }
    randomizer->counter = 0;
    
    QueryPerformanceFrequency(&randomizer->frequency);
    QueryPerformanceCounter(&randomizer->lastClickTime);
    
    // Initialize human behavioral state
    randomizer->concentration = 0.85f + (float)(randomizer->entropy % 100) / 1000.0f; // 0.85-0.95
    randomizer->fatigue = 0.0f;
    randomizer->rhythm = 1.0f;
    randomizer->microPauseCounter = 0;
    
    // Anti-detection enhancements
    randomizer->exhaustionLevel = 0.0f;
    randomizer->outlierForced = 0;
    randomizer->kurtosisCounter = 0.0f;
    randomizer->needsBreak = false;
    randomizer->lastBreakTime = GetTickCount();
    
    // Recovery system
    randomizer->lastActiveTime = GetTickCount();
    randomizer->isRecovering = false;
    randomizer->recoveryRate = 1.0f + (float)(randomizer->entropy % 50) / 100.0f; // 1.0-1.5
    randomizer->wasInactive = false;
    
    // Session tracking
    randomizer->sessionStart = GetTickCount();
    randomizer->clickCount = 0;
    
    // Pattern prevention
    for (int i = 0; i < 16; i++) randomizer->recentIntervals[i] = 0.0f;
    randomizer->intervalIndex = 0;
    
    // Burst/drop system
    randomizer->modifierRemaining = 0;
    randomizer->cpsModifier = 0;
    
    // Clustering
    randomizer->inCluster = false;
    randomizer->clusterRemaining = 0;
    randomizer->clusterMultiplier = 1.0f;
    
    // Advanced state
    randomizer->baseVariation = 0.05f + (float)(randomizer->entropy % 50) / 2000.0f; // 0.05-0.075
    randomizer->momentaryBias = 0.0f;
}

static void updateHumanState(bool isActivelyClicking) {
    DWORD currentTime = GetTickCount();
    DWORD sessionDuration = currentTime - randomizer->sessionStart;
    DWORD timeSinceActive = currentTime - randomizer->lastActiveTime;
    
    if (isActivelyClicking) {
        // Fatigue recovery on resume
        if (randomizer->wasInactive && timeSinceActive > 2000) {
            float breakDuration = (float)timeSinceActive / 1000.0f;
            float fatigueReduction = fminf(0.8f, 0.3f + (breakDuration / 60.0f));
            
            randomizer->fatigue *= (1.0f - fatigueReduction);
            randomizer->exhaustionLevel *= (1.0f - fatigueReduction);
            
            if (breakDuration > 15.0f) {
                randomizer->sessionStart = currentTime - (DWORD)(fatigueReduction * 120000.0f);
            }
        }
        
        randomizer->lastActiveTime = currentTime;
        randomizer->isRecovering = false;
        randomizer->wasInactive = false;
        
        // Minimal fatigue accumulation
        float timeFactor = (float)sessionDuration / 600000.0f; // 10 minutes to reach max
        randomizer->fatigue = 0.1f * (1.0f - expf(-timeFactor * 0.3f));
        
        if (timeFactor > 3.0f) { // After 30 minutes
            float exhaustionTime = timeFactor - 3.0f;
            randomizer->exhaustionLevel = 0.03f * (1.0f - expf(-exhaustionTime * 0.2f));
        }
    } else {
        if (timeSinceActive > 2000) {
            randomizer->wasInactive = true;
            randomizer->isRecovering = true;
            
            float recoveryTime = (float)timeSinceActive / 1000.0f;
            float recovery = recoveryTime * 0.025f;
            
            randomizer->fatigue -= recovery;
            if (randomizer->fatigue < 0.0f) randomizer->fatigue = 0.0f;
            
            randomizer->exhaustionLevel -= recovery * 0.8f;
            if (randomizer->exhaustionLevel < 0.0f) randomizer->exhaustionLevel = 0.0f;
        }
    }
    
    // Natural concentration with realistic variation
    float baseConcentration = 0.85f - (randomizer->fatigue * 0.2f) - (randomizer->exhaustionLevel * 0.15f);
    
    // Add natural attention fluctuations
    float attentionWave = sinf((float)sessionDuration / 20000.0f * TWO_PI) * 0.06f;
    float microFocus = randomRange(-0.04f, 0.04f);
    
    randomizer->concentration = baseConcentration + attentionWave + microFocus;
    if (randomizer->concentration > 1.0f) randomizer->concentration = 1.0f;
    if (randomizer->concentration < 0.65f) randomizer->concentration = 0.65f;
    
    // Natural rhythm with realistic drift
    float rhythmDrift = randomRange(-0.01f, 0.01f);
    float fatigueRhythm = randomizer->fatigue * randomRange(-0.02f, 0.02f);
    
    randomizer->rhythm += rhythmDrift + fatigueRhythm;
    if (randomizer->rhythm < 0.88f) randomizer->rhythm = 0.88f;
    if (randomizer->rhythm > 1.12f) randomizer->rhythm = 1.12f;
    
    // Natural counters
    if (isActivelyClicking) {
        randomizer->outlierForced++;
        randomizer->microPauseCounter++;
        
        // Realistic micro-pauses based on concentration
        int pauseThreshold = (int)(200.0f + randomRange(-50.0f, 50.0f));
        if (randomizer->microPauseCounter > pauseThreshold) {
            float pauseChance = 0.03f + (randomizer->fatigue * 0.05f);
            if (randomFloat() < pauseChance) {
                randomizer->microPauseCounter = -(int)randomRange(1.0f, 3.0f);
            } else {
                randomizer->microPauseCounter = 0;
            }
        }
    }
}

static float calculateHumanCPS(bool isActivelyClicking) {
    updateHumanState(isActivelyClicking);
    
    float baseCPS = (float)leftClicker.cps;
    float modifiedCPS = baseCPS;
    
    // Natural human behavioral variation
    modifiedCPS *= randomizer->rhythm;
    modifiedCPS *= (0.85f + randomizer->concentration * 0.15f);
    
    // Light fatigue effects
    if (randomizer->exhaustionLevel > 0.01f) {
        modifiedCPS *= (1.0f - randomizer->exhaustionLevel * 0.4f);
    }
    
    // Natural micro-pauses
    if (randomizer->microPauseCounter < 0) {
        modifiedCPS *= 0.6f;
        randomizer->microPauseCounter++;
        return modifiedCPS;
    }
    
    // CRITICAL: Natural outliers with proper frequency - every 15-30 clicks
    if (randomizer->outlierForced > (15 + (int)randomRange(0.0f, 15.0f))) {
        float outlierType = randomFloat();
        float outlierStrength = randomFloat();
        
        if (outlierType < 0.35f) {
            // Slow outliers (35%)
            if (outlierStrength < 0.3f) {
                modifiedCPS *= randomRange(0.3f, 0.6f); // Strong slow
            } else {
                modifiedCPS *= randomRange(0.6f, 0.8f); // Mild slow
            }
        } else if (outlierType < 0.7f) {
            // Fast outliers (35%)
            if (outlierStrength < 0.3f) {
                modifiedCPS *= randomRange(1.6f, 2.0f); // Strong fast
            } else {
                modifiedCPS *= randomRange(1.2f, 1.6f); // Mild fast
            }
        } else {
            // Moderate variations (30%)
            modifiedCPS *= randomRange(0.85f, 1.15f);
        }
        
        randomizer->outlierForced = 0;
        goto bounds_check;
    }
    
    // Natural clustering with realistic patterns
    if (!randomizer->inCluster && randomFloat() < (0.06f + randomizer->fatigue * 0.02f)) {
        randomizer->inCluster = true;
        randomizer->clusterRemaining = (int)randomRange(2.0f, 6.0f);
        // More varied cluster speeds
        float clusterRoll = randomFloat();
        if (clusterRoll < 0.3f) {
            randomizer->clusterMultiplier = randomRange(0.8f, 0.9f); // Slow cluster
        } else if (clusterRoll < 0.6f) {
            randomizer->clusterMultiplier = randomRange(1.1f, 1.2f); // Fast cluster
        } else {
            randomizer->clusterMultiplier = randomRange(0.95f, 1.05f); // Normal cluster
        }
    }
    
    if (randomizer->inCluster) {
        modifiedCPS *= randomizer->clusterMultiplier;
        randomizer->clusterRemaining--;
        if (randomizer->clusterRemaining <= 0) {
            randomizer->inCluster = false;
        }
    }
    
    // Enhanced burst/drop system
    if (randomizer->modifierRemaining > 0) {
        float concentrationFactor = 0.7f + randomizer->concentration * 0.3f;
        float proposedCPS = modifiedCPS + (randomizer->cpsModifier * concentrationFactor);
        
        if (randomizer->cpsModifier > 0) {
            float maxCPS = baseCPS + (float)leftClicker.cpsSpike;
            if (proposedCPS > maxCPS) {
                randomizer->cpsModifier = (int)(maxCPS - modifiedCPS);
            }
        }
        modifiedCPS += randomizer->cpsModifier * concentrationFactor;
        randomizer->modifierRemaining--;
    } else {
        float randomVal = randomFloat() * 100.0f;
        float fatigueInfluence = 1.0f + (randomizer->fatigue * 0.3f);
        
        if (randomVal < leftClicker.dropChance * fatigueInfluence) {
            randomizer->cpsModifier = -(int)randomRange(1.0f, (float)leftClicker.cpsDrop + 1.0f);
            randomizer->modifierRemaining = (int)randomRange(1.0f, 3.0f);
            modifiedCPS += randomizer->cpsModifier;
        } else if (randomVal < (leftClicker.dropChance + leftClicker.spikeChance) * (1.8f - fatigueInfluence * 0.2f)) {
            int spike = (int)randomRange(1.0f, (float)leftClicker.cpsSpike + 1.0f);
            float maxCPS = baseCPS + (float)leftClicker.cpsSpike;
            if (modifiedCPS + spike > maxCPS) {
                spike = (int)(maxCPS - modifiedCPS);
                if (spike < 1) spike = 1;
            }
            randomizer->cpsModifier = spike;
            randomizer->modifierRemaining = (int)randomRange(1.0f, 3.0f);
            modifiedCPS += randomizer->cpsModifier;
        }
    }
    
    // Multi-layered natural variation for proper variance
    float primaryVariation = gaussianRandom(0.0f, baseCPS * 0.015f);
    float secondaryVariation = randomRange(-baseCPS * 0.012f, baseCPS * 0.012f);
    float microVariation = gaussianRandom(0.0f, baseCPS * 0.008f);
    
    modifiedCPS += primaryVariation + secondaryVariation + microVariation;
    
    // Natural momentum/bias that creates realistic deviation patterns
    float momentumChange = randomRange(-0.015f, 0.015f);
    randomizer->momentaryBias += momentumChange;
    randomizer->momentaryBias *= 0.992f; // Slow decay
    
    // Apply bias with concentration influence
    float biasStrength = 1.0f - (randomizer->concentration * 0.3f);
    modifiedCPS += randomizer->momentaryBias * biasStrength;

bounds_check:
    // Natural bounds - not too restrictive
    float minCPS = baseCPS * 0.75f;
    float maxCPS = baseCPS + (float)leftClicker.cpsSpike;
    
    if (modifiedCPS < minCPS) modifiedCPS = minCPS;
    if (modifiedCPS > maxCPS) modifiedCPS = maxCPS;
    
    return modifiedCPS;
}

float getRandomInterval() {
    DWORD currentTimeMs = GetTickCount();
    bool isActivelyClicking = (currentTimeMs - randomizer->lastActiveTime) < HUMAN_ACTIVE_WINDOW_MS;
    
    float cps = calculateHumanCPS(isActivelyClicking);
    float baseInterval = 1000.0f / cps;
    
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    if (randomizer->lastClickTime.QuadPart != 0) {
        float actualInterval = (float)(currentTime.QuadPart - randomizer->lastClickTime.QuadPart) / randomizer->frequency.QuadPart * 1000.0f;
        randomizer->recentIntervals[randomizer->intervalIndex] = actualInterval;
        randomizer->intervalIndex = (randomizer->intervalIndex + 1) % 16;
        
        // Natural variance enhancement - if recent intervals are too similar
        float sum = 0, count = 0;
        for (int i = 0; i < 16; i++) {
            if (randomizer->recentIntervals[i] > 0) {
                sum += randomizer->recentIntervals[i];
                count++;
            }
        }
        
        if (count > 8) {
            float recentMean = sum / count;
            float variance = 0;
            for (int i = 0; i < 16; i++) {
                if (randomizer->recentIntervals[i] > 0) {
                    float diff = randomizer->recentIntervals[i] - recentMean;
                    variance += diff * diff;
                }
            }
            variance /= count;
            
            // If variance is too low, add natural human inconsistency
            if (variance < (recentMean * recentMean * 0.004f)) { // Less than 6.3% CV
                float varianceBoost = randomRange(-baseInterval * 0.2f, baseInterval * 0.2f);
                baseInterval += varianceBoost;
            }
        }
    }
    
    randomizer->lastClickTime = currentTime;
    
    // Enhanced natural timing variation
    float concentrationEffect = (1.0f - randomizer->concentration) * 0.05f;
    float fatigueEffect = randomizer->fatigue * 0.03f;
    float baseVariation = randomizer->baseVariation + concentrationEffect + fatigueEffect;
    
    // Multiple sources of natural variation
    float humanVariation = baseVariation * baseInterval;
    float attentionVariation = randomRange(-baseInterval * 0.04f, baseInterval * 0.04f);
    float muscleVariation = gaussianRandom(0.0f, baseInterval * 0.02f);
    
    float totalVariation = humanVariation + attentionVariation + muscleVariation;
    float finalInterval = gaussianRandom(baseInterval, totalVariation);
    
    if (finalInterval < CLICKER_MIN_INTERVAL_MS) finalInterval = CLICKER_MIN_INTERVAL_MS;
    
    return finalInterval;
}

float getRandomDuration() {
    float minDur = (float)leftClicker.minDuration;
    float maxDur = (float)leftClicker.maxDuration;
    float baseDuration = (minDur + maxDur) / 2.0f;
    float range = maxDur - minDur;
    
    // Moderate human behavioral influence
    float concentrationFactor = 0.85f + randomizer->concentration * 0.3f; // 0.85-1.15
    float fatigueFactor = 1.0f + randomizer->fatigue * 0.15f + randomizer->exhaustionLevel * 0.1f; // 1.0-1.25
    
    baseDuration *= concentrationFactor * fatigueFactor;
    
    // Natural variation with Gaussian distribution
    float variation = range * 0.25f;
    float duration = gaussianRandom(baseDuration, variation);
    
    // Ensure bounds
    if(duration < minDur) duration = minDur;
    if(duration > maxDur) duration = maxDur;
    
    return duration;
}

void leftClickerHandler() {
    static DWORD lastClickTime = 0;
    DWORD currentTime = GetTickCount();

    float nextInterval = getRandomInterval();

    if (currentTime - lastClickTime >= (DWORD)nextInterval) {
        float clickDuration = getRandomDuration();

        sendPostMessageA(true);

        robustSleep((DWORD)clickDuration / 1000.0);

        if(!globalSettings.breakBlocks) sendPostMessageA(false);

        lastClickTime = currentTime;
        randomizer->clickCount++;
    }
}