#include "resources/include.c"

#include "utils/utils.h"
#include "clicker/clicker.h"
#include "player/player.h"
#include "recorder/recorder.h"

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
    RecorderConfig recorder = {0};
    PlayerConfig* playerConfig = NULL;
    PlayerState playerState;
    RandomState randState;
    WavCollection soundCollection = {0};

    initGlobalConfig(&config);
    initClickerConfig(&clicker);
    initRandomState(&randState);
    initPlayerState(&playerState);

    int clickerMode;
    char recorderTestInput;

    clearScreen();

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
            printf("2. Enter Raw config\n");
            printf("3. Butterfly Click Profile\n");
            printf("4. Jitter Click Profile\n\n");

            printf("Input your choice: ");
            scanf_s("%d", &choice);

            switch (choice) {
                case 1:
                    if (playerConfig) freePlayerConfig(playerConfig);
                    {
                        char* filename = openConfigFileDialog();
                        if (filename) {
                            playerConfig = loadPlayerConfig(filename);
                            free(filename);
                        }
                    }
                    break;

                case 2:
                    fflush(stdin);
                    {
                        clearScreen();
                        printf("\nEnter raw config: ");
                        char* rawConfig = malloc(10485760); // 10MB buffer
                        if (rawConfig) {
                            if (fgets(rawConfig, 10485760, stdin) != NULL) {
                                rawConfig[strcspn(rawConfig, "\n")] = 0;
                                if (strlen(rawConfig) > 0) {
                                    if (playerConfig) freePlayerConfig(playerConfig);
                                    playerConfig = loadPlayerConfig(rawConfig);
                                }
                            }
                            free(rawConfig);
                        }
                    }
                    break;

                case 3:
                    if (playerConfig) freePlayerConfig(playerConfig);
                    playerConfig = loadPlayerConfig("butterfly");
                    break;

                case 4:
                    if (playerConfig) freePlayerConfig(playerConfig);
                    playerConfig = loadPlayerConfig("jitter");
                    break;
                    
                default:
                    printf("Invalid choice. Please select a valid option.\n");
                    break;
            }

            if (playerConfig) {
                clearScreen();
                printf("\nConfig Name: %s\n", playerConfig->configName);
                printf("Clicks: %d\n", playerConfig->clickCount);
                printf("Average CPS: %.2f\n", playerConfig->averageCPS);
            }
            break;

        case 3:
            clearScreen();
            
            printf("Bind Key for Recording (default: I): ");
            scanf_s(" %c", &recorder.bindKey);
            if (recorder.bindKey == '\0') recorder.bindKey = 'I';

            fflush(stdin);

            char beepInput;
            printf("Beep on Start/End? (Y or N): ");
            scanf_s(" %c", &beepInput);
            recorder.beepOnStart = (beepInput == 'Y' || beepInput == 'y');

            fflush(stdin);

            char mcOnlyInput;
            printf("Minecraft Only Recording? (Y or N): ");
            scanf_s(" %c", &mcOnlyInput);
            recorder.mcOnly = (mcOnlyInput == 'Y' || mcOnlyInput == 'y');

            fflush(stdin);

            char* recordedConfig = recordClicks(&recorder);
            if (recordedConfig) {
                printf("\nWould you like to test the recorded config? (Y/N): ");

                scanf_s(" %c", &recorderTestInput);

                if (recorderTestInput == 'Y' || recorderTestInput == 'y') {
                    if (playerConfig) freePlayerConfig(playerConfig);
                    playerConfig = loadPlayerConfig(recordedConfig);
                    if (playerConfig) {
                        config.playerActive = true;
                        config.leftActive = false;
                        printf("Config loaded for testing!\n");
                    }
                }
                free(recordedConfig);
            }
            break;

        default:
            printf("Invalid mode selected. Please choose 1, 2, 3.\n");
            break;
    }


    if (clickerMode != 3 || (clickerMode == 3 && (recorderTestInput == 'Y' || recorderTestInput == 'y'))) {
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
    } else {
        return 0;
    }


    while (true) {
        HWND currentWindow = GetForegroundWindow();
        HWND minecraftRecent = FindWindowA("GLFW30", NULL); // > 1.13
        HWND minecraftOld = FindWindowA("LWJGL", NULL); // < 1.13
        HWND minecraftBedrock = FindWindowA("ApplicationFrameWindow", NULL); // Bedrock Edition

        DWORD currentTime = GetTickCount();

        if (config.mcOnly) {
            if (currentWindow != minecraftRecent && 
                currentWindow != minecraftOld && 
                currentWindow != minecraftBedrock) {
                precisionSleep(1.0);
            }
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
                precisionSleep(durationClick);

                if (!config.breakBlocks) {
                    sendLeftClickDown(false);
                }

                precisionSleep(delayAfterClick);
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

            precisionSleep(1.0);
        }


        // Player Logic
        if (config.playerActive && playerConfig && playerConfig->clickCount > 0 && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clickInventory || !cursorVisible()) {

                // Soundclicks
                if (config.soundClicks) {
                    DWORD soundSize;
                    char* soundData = getRandomWavData(&soundCollection, &soundSize);
                    if (soundData) {
                        PlaySoundA(soundData, NULL, SND_MEMORY | SND_SYSTEM | SND_NOSTOP | SND_ASYNC);
                    }
                }

                // Initialize playback on first click or restart with new random position
                if (!playerState.isPlaying) {
                    playerState.isPlaying = true;
                    
                    // If we weren't playing last frame, set a new random start position
                    if (!playerState.wasPlayingLastFrame) {
                        setRandomStartPosition(&playerState, playerConfig->clickCount);
                    }
                    
                    QueryPerformanceCounter(&playerState.lastClickTime);
                }

                // Check if it's time for the next click
                LARGE_INTEGER currentTime;
                QueryPerformanceCounter(&currentTime);
                double timeSinceLastClick = ((double)(currentTime.QuadPart - playerState.lastClickTime.QuadPart) * 1000.0) / (double)playerState.frequency.QuadPart;
                
                // Get current click from config
                ClickData* click = &playerConfig->clicks[playerState.currentIndex];
                
                // For the first click or if enough time has passed
                bool shouldClick = false;
                if (playerState.currentIndex == 0 || !playerState.wasPlayingLastFrame) {
                    shouldClick = true; // First click or restart executes immediately
                } else {
                    // Check if enough time has passed since the last click
                    if (timeSinceLastClick >= click->delay) {
                        shouldClick = true;
                    }
                }
                
                if (shouldClick) {
                    sendLeftClickDown(true);
                    precisionSleep(click->duration);

                    if (!config.breakBlocks) {
                        sendLeftClickDown(false);
                    }
                    
                    // Update timing
                    QueryPerformanceCounter(&playerState.lastClickTime);
                    
                    // Move to next click
                    playerState.currentIndex++;
                    
                    if (playerState.currentIndex >= playerConfig->clickCount) {
                        playerState.currentIndex = 0;
                    }
                }
                
                // Mark that we were playing this frame
                playerState.wasPlayingLastFrame = true;
            }
        } else if (config.playerActive) {
            if (playerState.isPlaying) {
                resetPlayerState(&playerState);
            }
            // Mark that we weren't playing this frame
            playerState.wasPlayingLastFrame = false;
            PlaySoundA(NULL, NULL, SND_PURGE);
            precisionSleep(1.0);
        }
    }

    return 0;
}