#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

typedef struct configCauto {
    bool ativo;
    bool clicarInventario;
    bool quebrarBlocos;

    int inputCPS;
    float durMinClique;
    float durMaxClique;

    int chanceDrop;
    int chanceSpike;
    int cpsDrop;
    int cpsSpike;

    float variacaoClique;
} configCauto;

void init_config(configCauto *config);
float randomizar(float min, float max);
bool cursorVisivel(void);
void enviarClique(bool down);

#endif // UTILS_H