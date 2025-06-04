#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>  // For PROCESSENTRY32

int main() {
    HWND lastForeground = NULL;
    char lastTitle[256] = {0};
    char lastClass[256] = {0};
    
    printf("Active Window Monitor - Press Ctrl+C to exit\n");
    printf("===========================================\n");
    
    while (1) {
        HWND foreground = GetForegroundWindow();
        
        if (foreground && (foreground != lastForeground)) {
            char className[256] = {0};
            char windowTitle[256] = {0};
            DWORD processID = 0;
            char processName[MAX_PATH] = {0};
            char timeStr[20];
            time_t now;
            struct tm *tm_info;
            
            // Get current time
            time(&now);
            tm_info = localtime(&now);
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);
            
            // Get window class
            GetClassNameA(foreground, className, sizeof(className));
            
            // Get window title
            GetWindowTextA(foreground, windowTitle, sizeof(windowTitle));
            
            // Only update if window actually changed
            if (strcmp(windowTitle, lastTitle) != 0 || strcmp(className, lastClass) != 0 || !lastForeground) {
                // Get process ID
                GetWindowThreadProcessId(foreground, &processID);
                
                // Get process name using Toolhelp API
                HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (hSnapshot != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32 pe32;
                    pe32.dwSize = sizeof(PROCESSENTRY32);
                    
                    if (Process32First(hSnapshot, &pe32)) {
                        do {
                            if (pe32.th32ProcessID == processID) {
                                strncpy(processName, pe32.szExeFile, sizeof(processName));
                                break;
                            }
                        } while (Process32Next(hSnapshot, &pe32));
                    }
                    CloseHandle(hSnapshot);
                }
                
                // Print window information
                printf("\n[%s] Active Window Changed:\n", timeStr);
                printf("Handle:    0x%p\n", foreground);
                printf("Class:     %s\n", className);
                printf("Title:     %s\n", windowTitle);
                printf("Process:   %s\n", processName);
                printf("ProcessID: %d\n", processID);
                printf("--------------------------------------------------\n");
                
                // Update last window info
                lastForeground = foreground;
                strncpy(lastTitle, windowTitle, sizeof(lastTitle));
                strncpy(lastClass, className, sizeof(lastClass));
            }
        }
        
        Sleep(100);  // Check every 100ms
    }
    
    return 0;
}