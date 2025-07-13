#include "resources/include.c"

#include "utils/utils.h"
#include "utils/utils.c"

#include "clicker/clicker.h"
#include "clicker/clicker.c"

#include "player/player.h"
#include "player/player.c"

#include "recorder/recorder.h"
#include "recorder/recorder.c"

#include "utils/hwid.c"

int main() {
    // char HWIDListURL[] = "resources/hwidlist.txt";

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
    clickRecorder recorder;
    PlayerConfig* playerConfig = NULL;  // Pointer to PlayerConfig
    RandomState randState;
    WavCollection soundCollection = {0};

    initGlobalConfig(&config);
    initClickerConfig(&clicker);
    initRandomState(&randState);

    int mode;

    clearScreen();
    printf("Select the desired mode:\n\n");

    printf("1. Standard Clicker\n");
    printf("2. Click Player (RECOMMENDED)\n");
    printf("3. Click Recorder\n\n");

    printf("Input your choice: ");
    scanf_s("%d", &mode);

    switch (mode) {
        case 1:
            config.playerActive = false;
            config.leftActive = true;

            clearScreen();
            printf("Desired CPS: ");
            scanf_s("%d", &clicker.inputCPS);

            if(clicker.inputCPS < 1) {
                printf("Your CPS needs to be a value higher than 0.\n");
                return 1;
            }

            printf("\nSelect the minimum click duration (ms): ");
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

            printf("\nCPS Drop chance (%%): ");
            scanf_s("%f", &clicker.dropChance);
            if(clicker.dropChance < 0 || clicker.dropChance > 100) {
                printf("The drop chance must be between 0 and 100.\n");
                return 1;
            }

            printf("Amount of CPS in the DROP: ");
            scanf_s("%d", &clicker.dropCPS);

            printf("\nCPS Spike chance (%%): ");
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

                clearScreen();
                printf("1. Select a config file\n");
                printf("2. Select a Raw config\n");
                printf("3. Butterfly Click Profile (10k Clicks)\n");
                printf("4. Jitter Click Profile (10k Clicks)\n\n");

                printf("Input your choice: ");
                scanf_s("%d", &choice);

                switch (choice) {
                    case 1:
                        if (playerConfig) freePlayerConfig(playerConfig);
                        playerConfig = getPlayerConfig(false, NULL);  // Store the returned config
                        if (playerConfig) {
                            clearScreen();
                            printf("\nConfig Name: %s\n", playerConfig->configName);
                            printf("Clicks: %d (%d Double Clicks)\n", playerConfig->unifiedClicks, playerConfig->doubleClicks);
                            printf("Average CPS: %.2f\n\n", playerConfig->averageCPS);
                        }
                        break;

                    case 2:
                        {
                            clearScreen();
                            printf("\nEnter raw config: ");
                            
                            fflush(stdin);

                            char* rawConfig = malloc(1000000); // 1MB for large configs
                            if (!rawConfig) {
                                printf("Error: Memory allocation failed\n");
                                break;
                            }
                            
                            if (fgets(rawConfig, 100000, stdin) != NULL) {
                                rawConfig[strcspn(rawConfig, "\n")] = 0;
                                
                                if (strlen(rawConfig) > 0) {
                                    if (playerConfig) freePlayerConfig(playerConfig);
                                    playerConfig = getPlayerConfig(true, rawConfig);  // Store the returned config
                                    if (playerConfig) {
                                        clearScreen();
                                        printf("\nConfig Name: %s\n", playerConfig->configName);
                                        printf("Clicks: %d (%d Double Clicks)\n", playerConfig->unifiedClicks, playerConfig->doubleClicks);
                                        printf("Average CPS: %.2f\n\n", playerConfig->averageCPS);
                                    }
                                } else {
                                    printf("No config data entered.\n");
                                }
                            } else {
                                printf("Error reading config data.\n");
                            }
                            
                            free(rawConfig);
                        }
                        break;

                    case 3:
                        if (playerConfig) free(playerConfig);
                        getPlayerConfig(true, ButterflyConfig);
                        break;

                    case 4:
                        if (playerConfig) free(playerConfig);
                        getPlayerConfig(true, JitterConfig);
                        break;
                        
                    default:
                        printf("Invalid choice. Please select a valid option.\n");
                        break;
                }
            break;

        case 3:
            clearScreen();
            
            printf("Bind Key for Recording (default: INSERT): ");
            scanf_s(" %c", &recorder.bindKey);

            fflush(stdin);

            printf("Beep on Start/End? (Y or N): ");
            scanf_s(" %c", &recorder.beepOnStart);

            fflush(stdin);

            printf("Minecraft Only Recording? (Y or N): ");
            scanf_s(" %c", &recorder.mcOnly);

            fflush(stdin);

            recordClicks(&recorder);
            break;

        default:
            printf("Invalid mode selected. Please choose 1, 2, 3.\n");
            break;
    }


    // SoundClicks selector
    if (config.soundClicks) {
        printf("\nSelect WAV sound files for clicks...\n");
        
        if (openWavFileDialog(&soundCollection)) {
            printf("Loaded %d WAV file(s) successfully.\n", soundCollection.count);
        } else {
            printf("No WAV files selected. Sound will be disabled.\n");
            config.soundClicks = false;
        }
    }


    while (true) {
        HWND currentWindow = GetForegroundWindow();
        
        static HWND minecraftRecent = NULL;
        static HWND minecraftOld = NULL;
        static HWND minecraftBedrock = NULL;
        static DWORD lastWindowCheck = 0;
        
        DWORD currentTime = GetTickCount();
        if (currentTime - lastWindowCheck > 1000) {
            minecraftRecent = FindWindowA("GLFW30", NULL);
            minecraftOld = FindWindowA("LWJGL", NULL);
            minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL);
            lastWindowCheck = currentTime;
        }

        if (config.mcOnly &&
            currentWindow != minecraftRecent &&
            currentWindow != minecraftOld &&
            currentWindow != minecraftBedrock) {
            Sleep(50);
        }

        // Clicker Logic
        if (config.leftActive && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clickInventory || !cursorVisible()) {

                // Soundclicks
                if (config.soundClicks) {
                    DWORD soundSize;
                    char* soundData = getRandomWavData(&soundCollection, &soundSize);
                    if (soundData) {
                        PlaySoundA(soundData, NULL, SND_MEMORY | SND_SYSTEM | SND_NOSTOP | SND_ASYNC);
                    }
                }

                float durationClick = getClickDuration(&clicker, &randState);
                float clickInterval = getClickInterval(&clicker, &randState);
                
                float delayAfterClick = clickInterval - durationClick;
                if (delayAfterClick < 5) delayAfterClick = 5;

                sendLeftClickDown(true);
                Sleep((int)durationClick);

                if (!config.breakBlocks) {
                    sendLeftClickDown(false);
                }

                Sleep((int)delayAfterClick);
            }
        } else if (config.leftActive) {
            PlaySoundA(NULL, NULL, SND_PURGE);
            
            // Reduce calculation frequency when idle
            static DWORD lastIdleUpdate = 0;
            if (currentTime - lastIdleUpdate > 100) {
                DWORD timeSinceActive = currentTime - randState.human.lastActiveTime;
                if (timeSinceActive > 2000) {
                    float currentFatigue = randState.human.fatigue;
                    float currentExhaustion = randState.human.exhaustionLevel;
                    
                    getClickInterval(&clicker, &randState);
                    
                    if (randState.human.fatigue > currentFatigue) {
                        randState.human.fatigue = currentFatigue;
                    }
                    if (randState.human.exhaustionLevel > currentExhaustion) {
                        randState.human.exhaustionLevel = currentExhaustion;
                    }
                }
                lastIdleUpdate = currentTime;
            }
            Sleep(1);
        }


        // Player Logic
        if (config.playerActive && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clickInventory || !cursorVisible()) {

                // Soundclicks
                if (config.soundClicks) {
                    DWORD soundSize;
                    char* soundData = getRandomWavData(&soundCollection, &soundSize);
                    if (soundData) {
                        PlaySoundA(soundData, NULL, SND_MEMORY | SND_SYSTEM | SND_NOSTOP | SND_ASYNC);
                    }
                }

                // for (int i = rand() % (player.clickCount - 1 + 1) + 1; i < player.clickCount; i++) {
                        //sendLeftClickDown(true);
                        //Sleep((int)player.clicks[i].duration);

                        //if (!config.breakBlocks) {
                            //sendLeftClickDown(false);
                        //}

                //        Sleep((int)player.clicks[i].delay);
                // }
            }
        } else if (config.playerActive) {
            PlaySoundA(NULL, NULL, SND_PURGE);
            Sleep(1);
        }
    }

    return 0;
}