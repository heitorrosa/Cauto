#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "include/utils.h"
#include "include/utils.c"

int main() {
    configCauto config;
    RandomState randState;
    
    init_config(&config);
    init_random_state(&randState);

    printf("Desired CPS: ");
    scanf("%d", &config.inputCPS);

    if(config.inputCPS < 1) {
        printf("Your CPS needs to be a value higher than 0.\n");
        return 1;
    }

    printf("Select the minimum click duration (ms) / Default: 22ms: ");
    scanf("%f", &config.minDurationClick);
    if(config.minDurationClick < 10) {
        printf("The click duration must be greater than 10.\n");
        return 1;
    }

    printf("Select the maximum click duration (ms) / Default: 30ms: ");
    scanf("%f", &config.maxDurationClick);
    if(config.maxDurationClick < config.minDurationClick) {
        printf("The maximum click duration must be greater than the minimum.\n");
        return 1;
    }

    printf("CPS Drop chance (%%) / Default: 50%%: ");
    scanf("%f", &config.dropChance);
    if(config.dropChance < 0 || config.dropChance > 100) {
        printf("The drop chance must be between 0 and 100.\n");
        return 1;
    }

    printf("Amount of CPS in the DROP / Default: 3: ");
    scanf("%d", &config.dropCPS);

    printf("CPS Spike chance (%%) / Default: 50%%: ");
    scanf("%f", &config.spikeChance);
    if(config.spikeChance < 0 || config.spikeChance > 100) {
        printf("The spike chance must be between 0 and 100.\n");
        return 1;
    }

    printf("Amount of CPS in the SPIKE / Default: 2: ");
    scanf("%d", &config.spikeCPS);

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

                // Use enhanced randomization for click duration
                float durationClick = enhanced_randomization(config.minDurationClick, config.maxDurationClick, &randState);

                // Calculate CPS with burst sequences using only Spikes and Drops
                float modifiedCPS = calculate_cps_with_bursts(&config, &randState);

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