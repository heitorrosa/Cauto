#include "../resources/include.c"
#include "player.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

// Built-in configs (simplified for example)
static const char* ButterflyConfig = "";
static const char* JitterConfig = "";

void initPlayerState(PlayerState* state) {
    memset(state, 0, sizeof(PlayerState));
    state->currentIndex = 0;
    state->isPlaying = false;
    QueryPerformanceFrequency(&state->frequency);
    QueryPerformanceCounter(&state->lastClickTime);
}

void resetPlayerState(PlayerState* state) {
    state->currentIndex = 0;
    state->isPlaying = false;
    QueryPerformanceCounter(&state->lastClickTime);
}

static char* decryptHexData(const char* hexData) {
    if (!hexData || strlen(hexData) < 10) return NULL;
    
    int hexLen = strlen(hexData);
    if (hexLen % 2 != 0) return NULL;
    
    int binLen = hexLen / 2;
    char* binData = malloc(binLen + 1);
    char* decrypted = malloc(binLen + 1);
    
    if (!binData || !decrypted) {
        free(binData);
        free(decrypted);
        return NULL;
    }
    
    // Convert hex to binary
    int actualLen = HexToBinary(hexData, binData);
    
    // Decrypt using XOR
    XOREncryptDecrypt(binData, decrypted, actualLen);
    decrypted[actualLen] = '\0';
    
    free(binData);
    return decrypted;
}

static PlayerConfig* parseConfigData(const char* data) {
    if (!data) return NULL;
    
    PlayerConfig* config = malloc(sizeof(PlayerConfig));
    if (!config) return NULL;
    
    memset(config, 0, sizeof(PlayerConfig));
    strcpy(config->configName, "Unknown Config");
    
    // Parse header
    char* configLine = strstr(data, "Config: ");
    if (configLine) {
        sscanf(configLine, "Config: %255[^\n]", config->configName);
    }
    
    char* clicksLine = strstr(data, "Clicks: ");
    if (clicksLine) {
        sscanf(clicksLine, "Clicks: %d", &config->clickCount);
    }
    
    char* cpsLine = strstr(data, "Average CPS: ");
    if (cpsLine) {
        sscanf(cpsLine, "Average CPS: %lf", &config->averageCPS);
    }
    
    if (config->clickCount <= 0 || config->clickCount > 100000) {
        free(config);
        return NULL;
    }
    
    // Allocate clicks array
    config->clicks = malloc(config->clickCount * sizeof(ClickData));
    if (!config->clicks) {
        free(config);
        return NULL;
    }
    
    // Parse click data - create a modifiable copy for strtok
    char* dataCopy = strdup(data);
    if (!dataCopy) {
        free(config->clicks);
        free(config);
        return NULL;
    }
    
    char* line = strtok(dataCopy, "\n");
    int clickIndex = 0;
    
    while (line && clickIndex < config->clickCount) {
        // Look for lines with format: "index, duration, delay"
        int index;
        double duration, delay;
        if (sscanf(line, "%d, %lf, %lf", &index, &duration, &delay) == 3) {
            if (clickIndex < config->clickCount) {
                config->clicks[clickIndex].duration = duration;
                config->clicks[clickIndex].delay = delay;
                clickIndex++;
            }
        }
        line = strtok(NULL, "\n");
    }
    
    free(dataCopy);
    
    // If we didn't get all clicks, adjust count
    config->clickCount = clickIndex;
    
    if (config->clickCount == 0) {
        free(config->clicks);
        free(config);
        return NULL;
    }
    
    return config;
}

PlayerConfig* loadPlayerConfig(const char* input) {
    if (!input) return NULL;
    
    char* configData = NULL;
    
    // Check if it's a built-in config
    if (strcmp(input, "butterfly") == 0) {
        configData = decryptHexData(ButterflyConfig);
    } else if (strcmp(input, "jitter") == 0) {
        configData = decryptHexData(JitterConfig);
    } else if (strlen(input) > 100 && strlen(input) % 2 == 0) {
        // Assume it's encrypted hex data
        configData = decryptHexData(input);
    } else if (access(input, F_OK) == 0) {
        // It's a file path
        FILE* file = fopen(input, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char* fileData = malloc(size + 1);
            if (fileData) {
                fread(fileData, 1, size, file);
                fileData[size] = '\0';
                
                // Try to decrypt if it looks like hex
                if (size > 100 && size % 2 == 0) {
                    configData = decryptHexData(fileData);
                    free(fileData);
                } else {
                    configData = fileData;
                }
            }
            fclose(file);
        }
    } else {
        // Assume it's raw config data
        configData = strdup(input);
    }
    
    if (!configData) return NULL;
    
    PlayerConfig* config = parseConfigData(configData);
    free(configData);
    
    return config;
}

void freePlayerConfig(PlayerConfig* config) {
    if (config) {
        free(config->clicks);
        free(config);
    }
}