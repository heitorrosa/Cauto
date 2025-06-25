#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "include/utils.h"
#include "include/utils.c"

#include "include/hwid.c"

int main() {
    char HWIDListURL[] = "include/hwidlist.txt";

    if(HWIDchecker(HWIDListURL) == -1) {
        printf("error: The HWID list did not load\n");
        return 1;
    } else if (HWIDchecker(HWIDListURL) == 0) {
        printf("error: HWID not found in the HWID list.\n");

        char currentHWID[64];
        getHWID(currentHWID, sizeof(currentHWID));

        printf("Your hwid is: %s\n", currentHWID);
        return 1;
    } else {
        printf("HWID found in the list, continuing...\n");
    }

    configCauto config;
    RandomState randState;
    
    init_config(&config);
    init_randomState(&randState);

    printf("\n=== RECOMMENDED SETTINGS ===\n");
    printf("For 13 CPS:\n");
    printf("  - Drop Chance: 25%% (frequent, natural drops)\n");
    printf("  - Drop Amount: 2 CPS (subtle reduction)\n");
    printf("  - Spike Chance: 15%% (occasional bursts)\n");
    printf("  - Spike Amount: 1 CPS (gentle increase)\n");
    printf("\nFor 18 CPS:\n");
    printf("  - Drop Chance: 30%% (very frequent drops)\n");
    printf("  - Drop Amount: 3 CPS (noticeable reduction)\n");
    printf("  - Spike Chance: 12%% (controlled bursts)\n");
    printf("  - Spike Amount: 2 CPS (moderate increase)\n");
    printf("\nNOTE: Drop frequency automatically increases during long periods\n");
    printf("of normal clicking to create more natural human-like patterns.\n");
    printf("================================\n\n");

    printf("Desired CPS: ");
    scanf_s("%d", &config.inputCPS);

    if(config.inputCPS < 1) {
        printf("Your CPS needs to be a value higher than 0.\n");
        return 1;
    }

    printf("Select the minimum click duration (ms): ");
    scanf_s("%f", &config.minDurationClick);
    if(config.minDurationClick < 10) {
        printf("The click duration must be greater than 10.\n");
        return 1;
    }

    printf("Select the maximum click duration (ms): ");
    scanf_s("%f", &config.maxDurationClick);
    if(config.maxDurationClick < config.minDurationClick) {
        printf("The maximum click duration must be greater than the minimum.\n");
        return 1;
    }

    printf("CPS Drop chance (%%): ");
    scanf_s("%f", &config.dropChance);
    if(config.dropChance < 0 || config.dropChance > 100) {
        printf("The drop chance must be between 0 and 100.\n");
        return 1;
    }

    printf("Amount of CPS in the DROP: ");
    scanf_s("%d", &config.dropCPS);

    printf("CPS Spike chance (%%): ");
    scanf_s("%f", &config.spikeChance);
    if(config.spikeChance < 0 || config.spikeChance > 100) {
        printf("The spike chance must be between 0 and 100.\n");
        return 1;
    }

    printf("Amount of CPS in the SPIKE: ");
    scanf_s("%d", &config.spikeCPS);

    while (config.active) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {

        HWND currentWindow = GetForegroundWindow();
        HWND minecraftRecent = FindWindowA("GLFW30", NULL); //1.13 - Recent
        HWND minecraftOld = FindWindowA("LWJGL", NULL); // Older -  1.12
        HWND minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL); // Bedrock Edition

        if (config.mcOnly && currentWindow != minecraftRecent && currentWindow != minecraftOld && currentWindow != minecraftBedrock) {
            Sleep(1);
        }
        
        else {
            if (config.clickInventory || !visibleCursor()) {

                float durationClick = randomization(config.minDurationClick, config.maxDurationClick, &randState);

                float modifiedCPS = cpsWithBursts(&config, &randState);

                float randomizedCPS = 1000.0f / modifiedCPS;
                float randomizedClick = randomizedCPS - durationClick;

                if (randomizedClick < 5) randomizedClick = 5;

                sendClick(true);
                Sleep((int)durationClick);

                if(!config.breakBlocks) {
                    sendClick(false);
                }

                Sleep((int)randomizedClick);
            }
        
        }

        } else {
            Sleep(1);
        }
    }
    return 0;
}
