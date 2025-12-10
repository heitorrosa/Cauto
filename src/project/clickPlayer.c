#include "common.h"

void xorDecrypt(char *data, int len, char key) {
    for(int i = 0; i < len; i++)
        data[i] ^= key;
}

int parseConfigData(const char *hexBuf, int hexLen, clickPlayer_config *config) {
    if(!hexBuf || hexLen <= 0) {
        return 0;
    }

    char buf[4096] = {0};
    for(int i = 0; i < hexLen; i += 2) {
        if(sscanf_s(hexBuf + i, "%2hhx", (unsigned char *)&buf[i / 2]) != 1) {
            return 0;
        }
    }

    xorDecrypt(buf, hexLen / 2, 0x47);

    int off = 0;
    
    char tempName[256] = {0};
    if(sscanf_s(buf + off, "%255s", tempName, 256) != 1) {
        return 0;
    }
    
    for(int i = 0; tempName[i]; i++) {
        if(!isalnum(tempName[i]) && tempName[i] != '_') {
            return 0;
        }
    }
    
    if(strcpy_s(config->configName, 256, tempName) != 0) {
        return 0;
    }
    off += strlen(config->configName) + 1;

    int clickCnt, avgCps;
    if(sscanf_s(buf + off, "%d,%d", &clickCnt, &avgCps) != 2) {
        return 0;
    }
    
    config->clickCout = clickCnt;
    config->averageCPS = avgCps;
    off += strcspn(buf + off, "\n") + 1;

    config->clickData = malloc(sizeof(*config->clickData) * config->clickCout);
    if(!config->clickData) {
        return 0;
    }

    for(int i = 0; i < config->clickCout; i++) {
        if(sscanf_s(buf + off, "%d,%d", 
                    &config->clickData[i].clickDuration,
                    &config->clickData[i].delayBetweenClicks) != 2) {
            free(config->clickData);
            config->clickData = NULL;
            return 0;
        }
        off += strcspn(buf + off, "\n") + 1;
    }
    
    return 1;
}

void loadConfigFromString(const char *hexStr, clickPlayer_config *config) {
    if(!config) {
        return;
    }

    free(config->configName);
    free(config->clickData);
    config->configName = NULL;
    config->clickData = NULL;

    if(!hexStr || strlen(hexStr) == 0) {
        return;
    }
    
    config->configName = malloc(256);
    if(!config->configName) {
        return;
    }

    if(!parseConfigData(hexStr, strlen(hexStr), config)) {
        free(config->configName);
        free(config->clickData);
        config->configName = NULL;
        config->clickData = NULL;
        return;
    }
}

void loadConfigFromClipboard(clickPlayer_config *config) {
    if(!config) {
        return;
    }

    free(config->configName);
    free(config->clickData);
    config->configName = NULL;
    config->clickData = NULL;

    if(!OpenClipboard(NULL)) {
        return;
    }
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if(!hData) {
        CloseClipboard();
        return;
    }

    char *hexBuf = (char *)GlobalLock(hData);
    if(!hexBuf) {
        CloseClipboard();
        return;
    }

    config->configName = malloc(256);
    if(!config->configName) {
        GlobalUnlock(hData);
        CloseClipboard();
        return;
    }

    if(!parseConfigData(hexBuf, strlen(hexBuf), config)) {
        free(config->configName);
        free(config->clickData);
        config->configName = NULL;
        config->clickData = NULL;
        GlobalUnlock(hData);
        CloseClipboard();
        return;
    }

    GlobalUnlock(hData);
    CloseClipboard();
}

void loadConfigFromFile(clickPlayer_config *config) {
    if(!config) {
        return;
    }

    free(config->configName);
    free(config->clickData);
    config->configName = NULL;
    config->clickData = NULL;

    OPENFILENAME ofn = {0};
    char path[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.lpstrFilter = "Config Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(!GetOpenFileName(&ofn)) {
        return;
    }

    FILE *file;
    if(fopen_s(&file, path, "r") != 0) {
        return;
    }

    char hexBuf[8192] = {0};
    int hexLen = 0;
    int c;
    while((c = fgetc(file)) != EOF && hexLen < 8191) {
        hexBuf[hexLen++] = (char)c;
    }
    fclose(file);

    if(hexLen == 0) {
        return;
    }

    config->configName = malloc(256);
    if(!config->configName) {
        return;
    }

    if(!parseConfigData(hexBuf, hexLen, config)) {
        free(config->configName);
        free(config->clickData);
        config->configName = NULL;
        config->clickData = NULL;
        return;
    }
}

void clickPlayerHandler() {
    static int clickIndex = 0;

    if(clickIndex >= clickPlayer.clickCout) {
        clickIndex = 0;
        return;
    }

    int clickInterval = clickPlayer.clickData[clickIndex].delayBetweenClicks;
    int clickDuration = clickPlayer.clickData[clickIndex].clickDuration;

    sendPostMessageA(true);
    robustSleep(clickDuration / 1000.0f);

    if(!globalSettings.breakBlocks)
        sendPostMessageA(false);

    robustSleep(clickInterval / 1000.0f);

    clickIndex++;
}