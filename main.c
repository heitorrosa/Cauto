#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include <libwebsocket.h>

#include "include/utils.h"
#include "include/utils.c"

int main() {
    configCauto config;
    init_config(&config);

    printf("Desired CPS: ");
    scanf("%d", &config.inputCPS);

    if(config.inputCPS < 1) {
        printf("Your CPS needs to be a value higher than 0.\n");
        return 1;
    }

    fflush(stdin);

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

    printf("Amount of CPS in the DROP / Default: 10: ");
    scanf("%d", &config.dropCPS);

    printf("CPS Spike chance (%%) / Default: 50%%: ");
    scanf("%f", &config.spikeChance);
    if(config.spikeChance < 0 || config.spikeChance > 100) {
        printf("The spike chance must be between 0 and 100.\n");
        return 1;
    }

    printf("Amount of CPS in the SPIKE / Default: 10: ");
    scanf("%d", &config.spikeCPS);

    // printf("Selecione a variacao da duracao do clique (%%) / Recomendado: 10%%: ");
    // scanf("%f", &config.variacaoClique);
    //  if(config.variacaoClique < 0) {
    //    printf("A variacao da duracao do clique deve ser maior que 0.\n");
    //    return 1;
    // }

    // const float CPS = 1000.0f / config.inputCPS;

    //printf("Press 'B' to toggle smart mode.\n");
    //printf("Press 'N' to toggle break blocks.\n");
    //printf("Press 'M' to toggle MC only mode.\n");

    while (config.active) {
        //if (GetAsyncKeyState('B') & 0x8000) {
        //    config.clickInventory = !config.clickInventory;
        //    printf("Smart Mode: %s\n", config.clickInventory ? "ON" : "OFF");
        //    Sleep(200);
        //}

        //if (GetAsyncKeyState('N') & 0x8000) {
        //    config.breakBlocks = !config.breakBlocks;
        //    printf("Break Blocks: %s\n", config.breakBlocks ? "ON" : "OFF");
        //    Sleep(200);
        //}

        //if (GetAsyncKeyState('M') & 0x8000) {
        //    config.mcOnly = !config.mcOnly;
        //    printf("MC Only Mode: %s\n", config.mcOnly ? "ON" : "OFF");
        //    Sleep(200);
        //}

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clickInventory || !visibleCursor()) {

                float durationClick = randomization(config.minDurationClick, config.maxDurationClick);

                float modifiedCPS = config.inputCPS;
                float randomChance = randomization(0, 100);

                if (randomChance < config.dropChance) {
                    modifiedCPS -= config.dropCPS;
                    if (modifiedCPS < 1) modifiedCPS = 1; // Minimum CPS
                } else if (randomChance < config.dropChance + config.spikeChance) {
                    modifiedCPS += config.spikeCPS;
                }

                float randomizedCPS = 1000.0f / modifiedCPS;
                float randomizedClick = randomizedCPS - durationClick;

                // float delayVariacao = randomizar(-(config.variacaoClique / 100.0f), (config.variacaoClique / 100.0f)) * CPS;
                // float cliqueRandomizado = CPS - duracaoClique + delayVariacao;

                if (randomizedClick < 5) randomizedClick = 5;


                    sendClick(true);
                    Sleep((int)durationClick);
                    if(!config.breakBlocks) {
                        sendClick(false);
                    }
                    Sleep((int)randomizedClick);
            }
        } else {
            Sleep(1);
        }
    }
    return 0;
}