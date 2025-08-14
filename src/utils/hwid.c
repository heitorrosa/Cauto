#include "hwid.h"

void getHWID(char* buffer, size_t bufferSize) {
    HKEY hKey;
    char machineGuid[MACHINE_GUID_BUFFER] = {0};
    DWORD dataSize = sizeof(machineGuid);
    
    // Get Machine GUID from registry (survives reinstalls, tied to motherboard)
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "SOFTWARE\\Microsoft\\Cryptography", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, 
                            (LPBYTE)machineGuid, &dataSize) == ERROR_SUCCESS) {
            // Use first 32 chars and format as HWID
            if (strlen(machineGuid) >= 32) {
                snprintf(buffer, bufferSize, "%.8s-%.4s-%.4s-%.4s-%.12s",
                        machineGuid, machineGuid + 8, machineGuid + 12, 
                        machineGuid + 16, machineGuid + 20);
                RegCloseKey(hKey);
                return;
            }
        }
        RegCloseKey(hKey);
    }
    
    // Fallback: CPU + Motherboard serial (more stable than volume serial)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    unsigned int hwHash = sysInfo.dwProcessorType ^ sysInfo.dwNumberOfProcessors;
    hwHash = hwHash * 31 + GetTickCount(); // Add some entropy
    
    snprintf(buffer, bufferSize, "%08x-%04x-%04x-%04x-%08x",
            hwHash,
            (hwHash >> 16) & 0xFFFF,
            (hwHash >> 8) & 0xFFFF,
            hwHash & 0xFFFF,
            hwHash ^ 0xDEADBEEF);
}


int HWIDchecker(char *HWIDListURL) {
    char currentHWID[HWID_STRING_MAX];
    getHWID(currentHWID, sizeof(currentHWID));

    FILE *file;
    char line[LINE_BUFFER_MAX];

    errno_t err = fopen_s(&file, HWIDListURL, "r");
    if (err != 0 || file == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (strcmp(currentHWID, line) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}