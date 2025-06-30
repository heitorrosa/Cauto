#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "include/utils.h"
#include "include/utils.c"

#include "clicker/clicker.h"
#include "clicker/clicker.c"

#include "include/hwid.c"

int main() {
    // char HWIDListURL[] = "resources/hwidlist.txt";
    char pathSoundClicks[256] = {0}; // Changed from pointer to array

    // if(HWIDchecker(HWIDListURL) == -1) {
    //     printf("error: The HWID list did not load\n");
    //     return 1;
    // } else if (HWIDchecker(HWIDListURL) == 0) {
    //     printf("error: HWID not found in the HWID list.\n");

    //     char currentHWID[64];
    //     getHWID(currentHWID, sizeof(currentHWID));

    //     printf("Your hwid is: %s\n", currentHWID);
    //     return 1;
    // } else {
    //     printf("HWID found in the list, continuing...\n");
    // }

    globalConfig config;
    clickerConfig clicker;
    RandomState randState;
    
    initGlobalConfig(&config);
    initClickerConfig(&clicker);
    initRandomState(&randState);

    int mode;

    printf("Select the desired mode:\n");
    printf("1. Standard Clicker\n");
    printf("2. Click Player (RECOMMENDED)\n");
    scanf_s("%d", &mode);

    switch (mode) {
        case 1:
            config.playerActive = false;
            config.leftActive = true;

            printf("Desired CPS: ");
            scanf_s("%d", &clicker.inputCPS);

            if(clicker.inputCPS < 1) {
                printf("Your CPS needs to be a value higher than 0.\n");
                return 1;
            }

            printf("Select the minimum click duration (ms): ");
            scanf_s("%f", &clicker.minDurationClick);
            if(clicker.minDurationClick < 10) {
                printf("The click duration must be greater than 10.\n");
                return 1;
            }

            printf("Select the maximum click duration (ms): ");
            scanf_s("%f", &clicker.maxDurationClick);
            if(clicker.maxDurationClick < clicker.minDurationClick) {
                printf("The maximum click duration must be greater than the minimum.\n");
                return 1;
            }

            printf("CPS Drop chance (%%): ");
            scanf_s("%f", &clicker.dropChance);
            if(clicker.dropChance < 0 || clicker.dropChance > 100) {
                printf("The drop chance must be between 0 and 100.\n");
                return 1;
            }

            printf("Amount of CPS in the DROP: ");
            scanf_s("%d", &clicker.dropCPS);

            printf("CPS Spike chance (%%): ");
            scanf_s("%f", &clicker.spikeChance);
            if(clicker.spikeChance < 0 || clicker.spikeChance > 100) {
                printf("The spike chance must be between 0 and 100.\n");
                return 1;
            }

            printf("Amount of CPS in the SPIKE: ");
            scanf_s("%d", &clicker.spikeCPS);

            break;

        case 2:
            config.playerActive = true;
            config.leftActive = false;

            int choice;

                printf("1. Input a config\n\n");
                printf("2. Butterfly Click Profile (10k Clicks)\n");
                printf("3. Jitter Click Profile (10k Clicks)\n");
                printf("Input your choice: ");
                scanf_s("%d", &choice);

                switch (choice) {
                    case 1:

                    case 2:

                    case 3:

                    default:
                        printf("Invalid choice. Please select a valid option.\n");
                        return 1;
                }

            break;

        default:
            printf("Invalid mode selected. Please choose 1 or 2.\n");
            return 1;
    }

    if (mode == 1) {

    }

    if (mode == 2) {
 
    }

    fflush(stdin);

    printf("Soundclicks path (leave empty for disabling it): ");
    scanf_s("%255s", pathSoundClicks, 256);

    while (true) {
        HWND currentWindow = GetForegroundWindow();
        HWND minecraftRecent = FindWindowA("GLFW30", NULL);
        HWND minecraftOld = FindWindowA("LWJGL", NULL);
        HWND minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL);

        if (config.mcOnly && currentWindow != minecraftRecent && currentWindow != minecraftOld && currentWindow != minecraftBedrock) {
            Sleep(1);
        }
        else {
            if (config.leftActive && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                if (config.clickInventory || !cursorVisible()) {

                    // Use adaptive randomization for click duration
                    float durationClick = adaptiveRandomization(
                        (clicker.minDurationClick + clicker.maxDurationClick) / 2.0f,
                        (clicker.maxDurationClick - clicker.minDurationClick) / 4.0f,
                        &randState
                    );
                            
                    // Ensure bounds
                    if (durationClick < clicker.minDurationClick) durationClick = clicker.minDurationClick;
                    if (durationClick > clicker.maxDurationClick) durationClick = clicker.maxDurationClick;

                    float modifiedCPS = cpsWithBursts(&clicker, &randState);
                    float baseInterval = 1000.0f / modifiedCPS;
                            
                    // Add jitter to prevent perfect timing
                    float jitter = calculateJitter(&randState);
                    float randomizedClick = baseInterval - durationClick + jitter;

                    if (randomizedClick < 5) randomizedClick = 5;

                    if(config.soundClicks) {
                        PlaySoundA((char *)pathSoundClicks, NULL, SND_NOSTOP | SND_ASYNC | SND_FILENAME | SND_ALIAS);
                    }

                    // Main clicking logic
                    sendLeftClickDown(true);
                    Sleep((int)durationClick);

                    if(!config.breakBlocks) {
                        sendLeftClickDown(false);
                    }

                    Sleep((int)randomizedClick);
                }
            } else {
                PlaySoundA(NULL, NULL, SND_PURGE);
                Sleep(1);
            }
            
            if (config.playerActive && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                config.leftActive = false;

                if (config.clickInventory || !cursorVisible()) {

                }
            } else {
                PlaySoundA(NULL, NULL, SND_PURGE);
                Sleep(1);
            }
        }
    }

    return 0;
}