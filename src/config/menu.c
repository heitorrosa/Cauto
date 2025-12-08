#include "common.h"

static int booleanSwitch(_Bool *variable, int input) {
    switch(input) {
        case 'Y': case 'y': *variable = true; break;
        case 'N': case 'n': *variable = false; break;
        default: break;
    }

    return 0;
}

void globalSettingsMenu() {
    fflush(stdin);

    printf("\n=== Global Settings ===");
    printf("\nMinecraft Only (Y or N): ");
    char input = getch();
    printf("%c", input);
    booleanSwitch(&globalSettings.mcOnly, input);

    printf("\nBreak Blocks (Y or N): ");
    input = getch();
    printf("%c", input);
    booleanSwitch(&globalSettings.breakBlocks, input);
    
    printf("\nClick in Inventory (Y or N): ");
    input = getch();
    printf("%c", input);
    booleanSwitch(&globalSettings.clickInventory, input);

    printf("\n\nJitter Chance: ");
    scanf_s("%d", &globalSettings.jitterChance);
    if(globalSettings.jitterChance < 0) globalSettings.jitterChance = 0;
    if(globalSettings.jitterChance > 100) globalSettings.jitterChance = 100;

    printf("Jitter Y: ");
    scanf_s("%d", &globalSettings.jitterY);

    printf("Jitter X: ");
    scanf_s("%d", &globalSettings.jitterX);


    printf("\nClick Sounds (Y or N): ");
    input = getch();
    printf("%c", input);
    booleanSwitch(&globalSettings.clickSounds, input);

     if(globalSettings.clickSounds) selectClickSounds();
}

void leftClickerMenu() {
    fflush(stdin);
    clearScreen();

    clickPlayer.enabled = false;
    leftClicker.enabled = true;

    printf("=== Left Clicker ===");
    printf("\nDesired CPS: ");
    scanf_s("%d", &leftClicker.cps);
    if(leftClicker.cps < 1) leftClicker.cps = 1;

    printf("Minimum Click Duration: ");
    scanf_s("%d", &leftClicker.minDuration);
    if(leftClicker.minDuration < 10) leftClicker.minDuration = 10;

    printf("Maximum Click Duration: ");
    scanf_s("%d", &leftClicker.maxDuration);
    if(leftClicker.maxDuration < leftClicker.minDuration) leftClicker.maxDuration = leftClicker.minDuration;


    printf("\nDrop Chance: ");
    scanf_s("%d", &leftClicker.dropChance);
    if(leftClicker.dropChance < 0) leftClicker.dropChance = 0;
    if(leftClicker.dropChance > 100) leftClicker.dropChance = 100;

    printf("Drop CPS: ");
    scanf_s("%d", &leftClicker.cpsDrop);
    if(leftClicker.cpsDrop < 0) leftClicker.cpsDrop = 0;
    if(leftClicker.cpsDrop > leftClicker.cps) leftClicker.cpsDrop = leftClicker.cps;


    printf("\nSpike Chance: ");
    scanf_s("%d", &leftClicker.spikeChance);
    if(leftClicker.spikeChance < 0) leftClicker.spikeChance = 0;
    if(leftClicker.spikeChance > 100) leftClicker.spikeChance = 100;

    printf("Spike CPS: ");
    scanf_s("%d", &leftClicker.cpsSpike);
    if(leftClicker.cpsSpike < 0) leftClicker.cpsSpike = 0;
    if(leftClicker.cpsSpike > leftClicker.cps) leftClicker.cpsSpike = leftClicker.cps;

    globalSettingsMenu();
}

void clickPlayerMenu() {
    fflush(stdin);
    clearScreen();

    clickPlayer.enabled = true;
    leftClicker.enabled = false;

    printf("=== Click Player ===");
    printf("\n1. Load config file");
    printf("\n2. Load config from Clipboard");
    
    printf("\n\n3. Jitter Click Config (13CPS)");
    printf("\n4. Butterfly Click Config (15CPS)");

    char input = getch();

    switch(input) {
        case '1':
            // Load Config from file
            break;

        case '2':
            // Load config from Clipboard
            break;

        case '3':
            // Load Config with the Jitter Click config
            break;

        case '4':
            // Load Config with the Butterfly Click config 
            break;

        default:
            clickPlayerMenu();
            break;
    }

    globalSettingsMenu();
}

void clickRecorderMenu() {
    fflush(stdin);
    clearScreen();

    printf("=== Click Recorder ===");
    printf("\nRecord Keybind: ");
    printf("%c", clickRecorder.keybind = getch());

    printf("\nBeep On Start (Y or N): ");
    char input = getch();
    printf("%c", input);
    booleanSwitch(&clickRecorder.beepOnStart, input);

    printf("\nMinecraft Only (Y or N): ");
    input = getch();
    printf("%c", input);
    booleanSwitch(&clickRecorder.mcOnly, input);

    printf("\n\nClick Duration Threashold: ");
    scanf_s("%d", &clickRecorder.clickDuration_threshold);

    printf("Delay Between Clicks Threashold: ");
    scanf_s("%d", &clickRecorder.delayBetweenClicks_threashold);

    globalSettingsMenu();
}

void displayConfigs(char *input) {
    fflush(stdin);
    clearScreen();

    printf("=== Cauto ===\n");

    switch(*input) {
        case '1':
            printf("CPS: %d CPS", leftClicker.cps);
            printf("\nMinimum Click Duration: %dms", leftClicker.minDuration);
            printf("\nMaximum Click Duration: %dms", leftClicker.maxDuration);

            printf("\n\nDrop Chance: %d%%", leftClicker.dropChance);
            printf("\nDrop CPS: %d CPS", leftClicker.cpsDrop);
            
            printf("\n\nSpike Chance: %d%%", leftClicker.spikeChance);
            printf("\nSpike CPS: %d CPS", leftClicker.cpsSpike);
            break;

        case '2':
            printf("Config Name: %s", clickPlayer.configName);
            printf("\nClick Cout: %d", clickPlayer.clickCout);
            printf("\nAverage CPS: %d CPS", clickPlayer.averageCPS);
            break;

        default:
            break;
    }

    if(*input != '3') {
        printf("\n\nMinecraft Only: %s", globalSettings.mcOnly ? "Yes" : "No");
        printf("\nBreak Blocks: %s", globalSettings.breakBlocks ? "Yes" : "No");
        printf("\nClick in Inventory: %s", globalSettings.clickInventory? "Yes" : "No");

        printf("\n\nJitter Chance: %d%%", globalSettings.jitterChance);
        printf("\nJitter X: %dpx", globalSettings.jitterX);
        printf("\nJitter Y: %dpx", globalSettings.jitterY);

        printf("\n\nClick Sounds: %s (%d Files)\n\n", globalSettings.clickSounds ? "Yes" : "No", clickSounds.fileCount);

        printf("Press M for the Configuration Menu.");
    }
}

void cliMenu() {
    fflush(stdin);
    clearScreen();

    printf("=== Cauto ===\n");
    printf("Select the desired mode:\n");

    printf("1. Left Clicker\n");
    printf("2. Click Player\n");
    printf("3. Click Recorder");

    char input = getch();

    switch(input) {
        case '1':
            leftClickerMenu();
            break;

        case '2':
            clickPlayerMenu();
            break;

        case '3':
            clickRecorderMenu();
            break;

        default:
            cliMenu();
            break;
    }

    displayConfigs(&input);
}