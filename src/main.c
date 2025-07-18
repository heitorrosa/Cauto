#include "resources/include.c"

#include "utils/utils.h"
#include "clicker/clicker.h"
#include "player/player.h"
#include "recorder/recorder.h"
#include "websocket_example.h"

// Forward declaration for HWID functions
void getHWID(char* buffer, size_t bufferSize);
int HWIDchecker(char *HWIDListURL);

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

    int clickerMode;
    int clickIndex = -1;

    clearScreen();

    websocket_example();

    printf("Select the desired mode:\n\n");

    printf("1. Standard Clicker\n");
    printf("2. Click Player (RECOMMENDED)\n");
    printf("3. Click Recorder\n\n");

    printf("Input your choice: ");
    scanf_s("%d", &clickerMode);

    switch (clickerMode) {
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

            fflush(stdin);

            printf("\nSelect the minimum click duration (ms): ");
            scanf_s("%f", &clicker.minDurationClick);
            if(clicker.minDurationClick < 10) {
                printf("The click duration must be greater than 10.\n");
                return 1;
            }

            fflush(stdin);

            printf("Select the maximum click duration (ms): ");
            scanf_s("%f", &clicker.maxDurationClick);
            if(clicker.maxDurationClick < clicker.minDurationClick) {
                printf("The maximum click duration must be greater than the minimum.\n");
                return 1;
            }

            fflush(stdin);

            printf("\nCPS Drop chance (%%): ");
            scanf_s("%f", &clicker.dropChance);
            if(clicker.dropChance < 0 || clicker.dropChance > 100) {
                printf("The drop chance must be between 0 and 100.\n");
                return 1;
            }

            fflush(stdin);

            printf("Amount of CPS in the DROP: ");
            scanf_s("%d", &clicker.dropCPS);

            fflush(stdin);

            printf("\nCPS Spike chance (%%): ");
            scanf_s("%f", &clicker.spikeChance);
            if(clicker.spikeChance < 0 || clicker.spikeChance > 100) {
                printf("The spike chance must be between 0 and 100.\n");
                return 1;
            }

            fflush(stdin);

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
                        playerConfig = getPlayerConfig(false, NULL);
                        if (playerConfig) {
                            clearScreen();
                            printf("\nConfig Name: %s\n", playerConfig->configName);
                            printf("Clicks: %d (%d Double Clicks)\n", playerConfig->unifiedClicks, playerConfig->doubleClicks);
                            printf("Average CPS: %.2f\n\n", playerConfig->averageCPS);
                        }
                        break;

                    case 2:
                            
                            fflush(stdin);

                        {
                            clearScreen();
                            printf("\nEnter raw config: ");
                            char* rawConfig = malloc(1000000);
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
                        playerConfig = getPlayerConfig(true, ButterflyConfig);
                        break;

                    case 4:
                        if (playerConfig) free(playerConfig);
                        playerConfig = getPlayerConfig(true, JitterConfig);
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


    // Global Config (Idk why it doesnt work with switch)

    char mcOnlyInput;
    char clickInventoryInput;
    char breakBlocksInput;
    
    fflush(stdin);

    printf("\nMinecraft Only (Y or N): ");
    scanf_s(" %c", &mcOnlyInput);
    if(mcOnlyInput == 'Y' || mcOnlyInput == 'y') {
        config.mcOnly = true;
    } else if(mcOnlyInput == 'N' || mcOnlyInput == 'n') {
        config.mcOnly = false;
    } else {
        printf("Invalid input. Defaulting to 'N'.\n");
        config.mcOnly = false;
    }

    fflush(stdin);

    printf("\nClick Inventory (Y or N): ");
    scanf_s(" %c", &clickInventoryInput);
    if(clickInventoryInput == 'Y' || clickInventoryInput == 'y') {
        config.clickInventory = true;
    } else if(clickInventoryInput == 'N' || clickInventoryInput == 'n') {
        config.clickInventory = false;
    } else {
        printf("Invalid input. Defaulting to 'N'.\n");
        config.clickInventory = false;
    }

    fflush(stdin);
    printf("\nBreak Blocks (Y or N): ");
    scanf_s(" %c", &breakBlocksInput);
    if(breakBlocksInput == 'Y' || breakBlocksInput == 'y') {
        config.breakBlocks = true;
    } else if(breakBlocksInput == 'N' || breakBlocksInput == 'n') {
        config.breakBlocks = false;
    } else {
        printf("Invalid input. Defaulting to 'Y'.\n");
        config.breakBlocks = true;
    }

    fflush(stdin);

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
        HWND minecraftRecent = FindWindowA("GLFW30", NULL); // > 1.13
        HWND minecraftOld = FindWindowA("LWJGL", NULL); // < 1.13
        HWND minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL); // Bedrock Edition

        DWORD currentTime = GetTickCount();

        if (config.mcOnly &&
            currentWindow != minecraftRecent ||
            currentWindow != minecraftOld ||
            currentWindow != minecraftBedrock) {
            Sleep(1);
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

                // Randomization
                float durationClick = getClickDuration(&clicker, &randState);
                float clickInterval = getClickInterval(&clicker, &randState);
                
                float delayAfterClick = clickInterval - durationClick;
                if (delayAfterClick < 5) delayAfterClick = 5;

                // Click Logic
                sendLeftClickDown(true);
                Sleep((int)durationClick);

                if (!config.breakBlocks) {
                    sendLeftClickDown(false);
                }

                Sleep((int)delayAfterClick);
            }
        } else if (config.leftActive) {
            PlaySoundA(NULL, NULL, SND_PURGE); // Stop any playing sound
            
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
        if (config.playerActive && playerConfig->clickCount > 0 && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clickInventory || !cursorVisible()) {

                // Soundclicks
                if (config.soundClicks) {
                    DWORD soundSize;
                    char* soundData = getRandomWavData(&soundCollection, &soundSize);
                    if (soundData) {
                        PlaySoundA(soundData, NULL, SND_MEMORY | SND_SYSTEM | SND_NOSTOP | SND_ASYNC);
                    }
                }


                // Random click selection
                if(clickIndex == -1 || clickIndex >= playerConfig->clickCount) {
                    clickIndex = rand() % playerConfig->clickCount;
                    printf("Starting playback from click %d/%d\n", clickIndex + 1, playerConfig->clickCount);
                }


                sendLeftClickDown(true);
                Sleep((int)playerConfig->clicks[clickIndex].duration);

                if (!config.breakBlocks) {
                    sendLeftClickDown(false);
                }

                Sleep((int)playerConfig->clicks[clickIndex].delay);

                // Update click index with the next click
                clickIndex = (clickIndex + 1) % playerConfig->clickCount;
            }
        } else if (config.playerActive) {
            clickIndex = -1; // Reset the click index
            PlaySoundA(NULL, NULL, SND_PURGE); // Stop any playing sound
            Sleep(1);
        }
    }

    return 0;
}