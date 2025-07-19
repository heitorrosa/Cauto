#include "../resources/include.c"
#include "player.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

// Config data definitions
char* ButterflyConfig = "";
char* JitterConfig = "";

static char* loadFileContents(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* data = malloc(size + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }
    
    fread(data, 1, size, file);
    data[size] = '\0';
    fclose(file);
    
    // Check if this is hex data (encrypted)
    if (size > 100 && size % 2 == 0) {
        // Try to decrypt as hex
        int binLen = size / 2;
        char* binData = malloc(binLen + 1);
        char* rawData = malloc(binLen + 1);
        
        if (binData && rawData) {
            HexToBinary(data, binData);
            XOREncryptDecrypt(binData, rawData, binLen);
            rawData[binLen] = '\0';
            
            free(data);
            free(binData);
            return rawData;
        }
        
        free(binData);
        free(rawData);
    }
    
    return data;
}

static char* loadRawConfig(const char* input, bool* isFromFile) {
    *isFromFile = false;
    
    if (strstr(input, "[CLICK_RECORDER_DATA]") || strlen(input) > 500) {
        if (strstr(input, "[CLICK_RECORDER_DATA]")) {
            return strdup(input);
        }
        
        int hexLen = strlen(input);
        if (hexLen % 2 == 0 && hexLen > 100) {
            int binLen = hexLen / 2;
            char* binData = malloc(binLen + 1);
            char* rawData = malloc(binLen + 1);
            
            if (!binData || !rawData) {
                free(binData);
                free(rawData);
                return NULL;
            }
            
            HexToBinary(input, binData);
            XOREncryptDecrypt(binData, rawData, binLen);
            rawData[binLen] = '\0';
            
            free(binData);
            return rawData;
        }
    }
    
    // Try to load as file path
    if (access(input, F_OK) == 0) {
        *isFromFile = true;
        return loadFileContents(input);
    }
    
    return NULL;
}

static ParsedClick* parseClickData(const char* data, int* count) {
    *count = 0;
    
    char* dataCopy = strdup(data);
    if (!dataCopy) return NULL;
    
    char* lines[10000];
    int lineCount = 0;
    char* token = strtok(dataCopy, "\n");
    
    while (token && lineCount < 10000) {
        if (strstr(token, "Click ID:")) {
            lines[lineCount++] = strdup(token);
        }
        token = strtok(NULL, "\n");
    }
    
    if (lineCount == 0) {
        free(dataCopy);
        return NULL;
    }
    
    ParsedClick* clicks = malloc(lineCount * sizeof(ParsedClick));
    if (!clicks) {
        for (int i = 0; i < lineCount; i++) {
            free(lines[i]);
        }
        free(dataCopy);
        return NULL;
    }
    
    for (int i = 0; i < lineCount; i++) {
        sscanf(lines[i], "Click ID: %d, Duration: %lf ms, Delay: %lf ms", 
               &clicks[i].id, &clicks[i].duration, &clicks[i].delay);
        free(lines[i]);
    }
    
    *count = lineCount;
    free(dataCopy);
    return clicks;
}

static PlayerConfig* parseConfig(const char* rawData) {
    if (!rawData) return NULL;
    
    PlayerConfig* config = malloc(sizeof(PlayerConfig));
    if (!config) return NULL;
    
    // Initialize defaults
    memset(config, 0, sizeof(PlayerConfig));
    strcpy(config->configName, "Unknown");
    
    // Parse header info - handle both old and new formats
    char* configLine = strstr(rawData, "config_name=");
    if (!configLine) {
        configLine = strstr(rawData, "Config Name:");
    }
    
    if (configLine) {
        if (strstr(configLine, "config_name=")) {
            // New format: config_name=Name
            sscanf(configLine, "config_name=%255[^\n]", config->configName);
        } else {
            // Old format: Config Name: Name
            sscanf(configLine, "Config Name: %255[^\n]", config->configName);
        }
    }
    
    char* clicksLine = strstr(rawData, "total_clicks=");
    if (!clicksLine) {
        clicksLine = strstr(rawData, "Total Clicks:");
    }
    
    if (clicksLine) {
        if (strstr(clicksLine, "total_clicks=")) {
            sscanf(clicksLine, "total_clicks=%d", &config->totalClicks);
        } else {
            sscanf(clicksLine, "Total Clicks: %d", &config->totalClicks);
        }
    }
    
    char* doubleClicksLine = strstr(rawData, "double_clicks=");
    if (!doubleClicksLine) {
        doubleClicksLine = strstr(rawData, "Double Clicks:");
    }
    
    if (doubleClicksLine) {
        if (strstr(doubleClicksLine, "double_clicks=")) {
            sscanf(doubleClicksLine, "double_clicks=%d", &config->doubleClicks);
        } else {
            sscanf(doubleClicksLine, "Double Clicks: %d", &config->doubleClicks);
        }
    }
    
    char* unifiedClicksLine = strstr(rawData, "unified_clicks=");
    if (!unifiedClicksLine) {
        unifiedClicksLine = strstr(rawData, "Unified Clicks:");
    }
    
    if (unifiedClicksLine) {
        if (strstr(unifiedClicksLine, "unified_clicks=")) {
            sscanf(unifiedClicksLine, "unified_clicks=%d", &config->unifiedClicks);
        } else {
            sscanf(unifiedClicksLine, "Unified Clicks: %d", &config->unifiedClicks);
        }
    }
    
    char* averageCPSLine = strstr(rawData, "average_cps=");
    if (!averageCPSLine) {
        averageCPSLine = strstr(rawData, "Average CPS:");
    }
    
    if (averageCPSLine) {
        if (strstr(averageCPSLine, "average_cps=")) {
            sscanf(averageCPSLine, "average_cps=%lf", &config->averageCPS);
        } else {
            sscanf(averageCPSLine, "Average CPS: %lf", &config->averageCPS);
        }
    }
    
    // Parse click data - handle new format with unified clicks
    char* unifiedSection = strstr(rawData, "[UNIFIED_CLICKS]");
    if (unifiedSection) {
        // New format: comma-separated values after [UNIFIED_CLICKS]
        char* dataStart = strchr(unifiedSection, '\n');
        if (dataStart) {
            dataStart++; // Skip the newline
            
            // Count clicks by counting commas + 1
            int clickCount = 1;
            char* temp = dataStart;
            while ((temp = strchr(temp, ',')) != NULL) {
                clickCount++;
                temp++;
            }
            
            config->clicks = malloc(clickCount * sizeof(ParsedClick));
            if (config->clicks) {
                config->clickCount = 0;
                
                char* dataCopy = strdup(dataStart);
                if (dataCopy) {
                    char* token = strtok(dataCopy, ",\n");
                    while (token && config->clickCount < clickCount) {
                        int id;
                        double duration, delay;
                        if (sscanf(token, "%d:%lf:%lf", &id, &duration, &delay) == 3) {
                            config->clicks[config->clickCount].id = id;
                            config->clicks[config->clickCount].duration = duration;
                            config->clicks[config->clickCount].delay = delay;
                            config->clickCount++;
                        }
                        token = strtok(NULL, ",\n");
                    }
                    free(dataCopy);
                }
            }
        }
    } else {
        // Try old format
        config->clicks = parseClickData(rawData, &config->clickCount);
    }
    
    return config;
}

PlayerConfig* getPlayerConfig(bool getRawConfig, const char* rawConfigData) {
    bool isFromFile = false;
    char* rawData = NULL;
    
    if (getRawConfig && rawConfigData) {
        rawData = loadRawConfig(rawConfigData, &isFromFile);
    } else {
        OPENFILENAME ofn;
        char szFile[260] = {0};
        
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        
        if (GetOpenFileName(&ofn)) {
            rawData = loadFileContents(szFile);
            isFromFile = true;
        }
    }
    
    if (!rawData) {
        return NULL;
    }
    
    PlayerConfig* config = parseConfig(rawData);
    free(rawData);
    
    return config;
}

void freePlayerConfig(PlayerConfig* config) {
    if (config) {
        if (config->clicks) {
            free(config->clicks);
        }
        free(config);
    }
}