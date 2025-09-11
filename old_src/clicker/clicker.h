#ifndef CLICKER_H
#define CLICKER_H

#include "../common/common.h"
#include "../utils/utils.h"

typedef struct {
    int inputCPS;

    float minDurationClick;
    float maxDurationClick;

    float dropChance;
    int dropCPS;
    
    float spikeChance;
    int spikeCPS;
} clickerConfig;

// Human behavioral state tracking
typedef struct {
    float concentration;    // 0.0-1.0, affects consistency
    float fatigue;         // 0.0-1.0, increases over time
    float rhythm;          // Current clicking rhythm bias
    int microPauseCounter; // Tracks when micro-pauses occur
    
    // Anti-detection features
    float exhaustionLevel;     // More severe fatigue
    int outlierForced;        // Force outliers periodically
    float kurtosisCounter;    // Track kurtosis manipulation
    bool needsBreak;          // Force breaks
    DWORD lastBreakTime;      // Track break timing
    
    // Recovery system
    DWORD lastActiveTime;     // Last time user was actively clicking
    bool isRecovering;        // Currently in recovery phase
    float recoveryRate;       // How fast fatigue recovers
    bool wasInactive;         // Track if user was previously inactive
} HumanState;

// Advanced entropy and timing state
typedef struct {
    // High-quality entropy sources
    unsigned long long entropy;     // 64-bit entropy accumulator
    unsigned int chacha_state[16];  // ChaCha20-like state for cryptographic quality
    unsigned int counter;           // Stream cipher counter
    
    // Human behavioral modeling
    HumanState human;
    
    // Burst sequence state (simplified)
    int remainingTicks;
    int cpsModifier;
    
    // Timing adaptation
    LARGE_INTEGER lastClickTime;
    LARGE_INTEGER frequency;
    float recentIntervals[16];      // Reduced history size
    int intervalIndex;
    
    // Natural variation patterns
    float baseVariation;            // Core timing inconsistency
    float momentaryBias;           // Short-term timing drift
    DWORD sessionStartTime;
    
    // Temporal clustering (humans tend to cluster clicks)
    bool inCluster;
    int clusterRemaining;
    float clusterSpeedModifier;
} RandomState;

// Core functions
void initRandomState(RandomState *state);
void initClickerConfig(clickerConfig *clicker);
float getClickInterval(clickerConfig *clicker, RandomState *state);
float getClickDuration(clickerConfig *clicker, RandomState *state);

#endif // CLICKER_H