#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

int HWIDchecker(char *HWIDListURL) {
    char HWID[] = "aaaaa13b-aaaaa-aaaaa-aaaaa-aaaaa";

    FILE *file;
    char line[256];

    file = fopen(HWIDListURL, "r");
    if (file == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (strcmp(HWID, line) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}