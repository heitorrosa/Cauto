#include "../resources/include.c"
#include "player.h"
#include "../utils/utils.h"
#include "../utils/crypto.h"

char* ButterflyConfig = "";
char* JitterConfig = "";


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
            
            hex_to_binary(input, binData);
            xor_encrypt_decrypt(binData, rawData, binLen);
            rawData[binLen] = '\0';
            
            free(binData);
            
            if (strstr(rawData, "[CLICK_RECORDER_DATA]")) {
                return rawData;
            } else {
                free(rawData);
                printf("Error: Invalid encrypted config data\n");
                return NULL;
            }
        }
        
        return strdup(input);
    }
    
    FILE* file = fopen(input, "r");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", input);
        return NULL;
    }
    
    // Read file
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* hexData = malloc(size + 1);
    if (!hexData) {
        fclose(file);
        return NULL;
    }
    
    fread(hexData, 1, size, file);
    hexData[size] = '\0';
    fclose(file);
    
    // Decrypt
    int binLen = size / 2;
    char* binData = malloc(binLen + 1);
    char* rawData = malloc(binLen + 1);
    
    if (!binData || !rawData) {
        free(hexData);
        free(binData);
        free(rawData);
        return NULL;
    }
    
    hex_to_binary(hexData, binData);
    xor_encrypt_decrypt(binData, rawData, binLen);
    rawData[binLen] = '\0';
    
    free(hexData);
    free(binData);
    *isFromFile = true;
    return rawData;
}

static void parseClickData(char* line, PlayerConfig* config) {
    int count = 1;
    for (int i = 0; line[i]; i++) if (line[i] == ',') count++;
    
    config->clicks = malloc(count * sizeof(ParsedClick));
    if (!config->clicks) {
        printf("Error: Could not allocate memory for clicks\n");
        return;
    }
    
    config->clickCount = 0;
    char* pos = line;
    
    for (int i = 0; i < count && pos; i++) {
        char* comma = strchr(pos, ',');
        char clickStr[128];
        
        // Extract individual click string
        if (comma) {
            strncpy(clickStr, pos, comma - pos);
            clickStr[comma - pos] = '\0';
            pos = comma + 1;
        } else {
            strcpy(clickStr, pos);
            pos = NULL;
        }
        
        // Parse id:duration:delay
        char* c1 = strchr(clickStr, ':');
        if (c1) {
            *c1++ = '\0';
            char* c2 = strchr(c1, ':');
            if (c2) {
                *c2++ = '\0';
                config->clicks[config->clickCount].id = atoi(clickStr);
                config->clicks[config->clickCount].duration = atof(c1);
                config->clicks[config->clickCount].delay = atof(c2);
                config->clickCount++;
            }
        }
    }
}

static bool parseConfig(char* rawData, PlayerConfig* config) {
    memset(config, 0, sizeof(PlayerConfig));
    
    char* line = strtok(rawData, "\n");
    bool inClicks = false;
    
    while (line) {
        if (strstr(line, "config_name=")) 
            strncpy(config->configName, line + 12, sizeof(config->configName) - 1);
        else if (strstr(line, "total_clicks=")) 
            config->totalClicks = atoi(line + 13);
        else if (strstr(line, "double_clicks=")) 
            config->doubleClicks = atoi(line + 14);
        else if (strstr(line, "unified_clicks=")) 
            config->unifiedClicks = atoi(line + 15);
        else if (strstr(line, "average_cps=")) 
            config->averageCPS = atof(line + 12);
        else if (strstr(line, "[UNIFIED_CLICKS]")) 
            inClicks = true;
        else if (inClicks && strlen(line) > 0) {
            parseClickData(line, config);
            break;
        }
        
        line = strtok(NULL, "\n");
    }
    
    return config->clickCount > 0;
}

static void displayConfigInfo(const PlayerConfig* config) {
    clearScreen();
    printf("\nConfig Name: %s\n", config->configName);
    printf("Clicks: %d (%d Double Clicks)\n", config->unifiedClicks, config->doubleClicks);
    printf("Average CPS: %.2f\n\n", config->averageCPS);
}

PlayerConfig* getPlayerConfig(bool getRawConfig, const char* rawConfigData) {
    PlayerConfig* config = malloc(sizeof(PlayerConfig));

    if (!config) {
        printf("Error: Could not allocate memory for config\n");
        return NULL;
    }
    
    char* rawData = NULL;
    bool isFromFile = false;
    
    if (getRawConfig) {
        if (!rawConfigData || strlen(rawConfigData) == 0) {
            printf("Error: No raw config data provided\n");
            free(config);
            return NULL;
        }
        

        rawData = loadRawConfig(rawConfigData, &isFromFile);
        if (!rawData) {
            printf("Error: Failed to process raw config data\n");
            free(config);
            return NULL;
        }
    } else {
        OPENFILENAMEA ofn = {0};
        char file[MAX_PATH] = {0};
        
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = file;
        ofn.nMaxFile = sizeof(file);
        ofn.lpstrFilter = "All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = "Select Click Configuration File";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        
        if (!GetOpenFileNameA(&ofn)) {
            printf("No file selected or dialog cancelled.\n");
            
            // Fallback to manual input
            printf("\nAlternatively, enter filename manually: ");
            fflush(stdout);
            
            char filename[MAX_PATH];
            if (fgets(filename, sizeof(filename), stdin) != NULL) {
                filename[strcspn(filename, "\n")] = 0;
                if (strlen(filename) > 0) {
                    strcpy(file, filename);
                } else {
                    printf("No filename provided.\n");
                    free(config);
                    return NULL;
                }
            } else {
                printf("No filename provided.\n");
                free(config);
                return NULL;
            }
        }
        
        printf("Selected file: %s\n\n", file);
        
        rawData = loadRawConfig(file, &isFromFile);
        if (!rawData) {
            free(config);
            return NULL;
        }
    }
    
    if (!parseConfig(rawData, config)) {
        printf("Error: Failed to parse config data\n");
        free(rawData);
        free(config);
        return NULL;
    }
    
    displayConfigInfo(config);
    
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