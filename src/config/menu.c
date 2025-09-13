#include "../include/common.h"

void globalSettingsMenu() {
    printf("\n\n=== Global Settings ===");

    printf("\nMinecraft Only (Y or N): ");
    char input = getch();
    printf("%c", input);
    if(input == 'Y' || input == 'y') globalSettings.mcOnly = true;
    if(input == 'N' || input == 'n') globalSettings.mcOnly = false;

    printf("\nBreak Blocks (Y or N): ");
    input = getch();
    printf("%c", input);
    if(input == 'Y' || input == 'y') globalSettings.breakBlocks = true;
    if(input == 'N' || input == 'n') globalSettings.breakBlocks = false;
    
    printf("\nClick in Inventory (Y or N): ");
    input = getch();
    printf("%c", input);
    if(input == 'Y' || input == 'y') globalSettings.clickInventory = true;
    if(input == 'N' || input == 'n') globalSettings.clickInventory = false;

    printf("\n\nJitter Y: ");
    scanf_s("%d", &globalSettings.jitterY);
    
    printf("Jitter X: ");
    scanf_s("%d", &globalSettings.jitterX);


    printf("\nClickSounds (Y or N): ");
    input = getch();
    printf("%c", input);
    if(input == 'Y' || input == 'y') globalSettings.clickSounds = true;
    if(input == 'N' || input == 'n') globalSettings.clickSounds = false;

    if(globalSettings.clickSounds) printf("select clicksounds");
}

void leftClickerMenu() {
    clearScreen();

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

}

void clickRecorderMenu() {

}

void cliMenu() {
    clearScreen();

    printf("=== Cauto Autoclicker ===\n");
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
}