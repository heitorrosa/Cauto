#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "include/utils.h"
#include "include/utils.c"

int main() {
    configCauto config;
    init_config(&config);

    // Configurações AutoClicker
    printf("CPS desejado: ");
    scanf("%d", &config.inputCPS);

    if(config.inputCPS < 1) {
        printf("O CPS deve ser maior que 0.\n");
        return 1;
    }

    fflush(stdin);

    printf("Selecione a minima duracao do clique (ms) / Recomendado: 22ms: ");
    scanf("%f", &config.durMinClique);
    if(config.durMinClique < 10) {
        printf("A duracao do clique deve ser maior que 10.\n");
        return 1;
    }

    printf("Selecione a maxima duracao do clique (ms) / Recomendado: 30ms: ");
    scanf("%f", &config.durMaxClique);
    if(config.durMaxClique < config.durMinClique) {
        printf("A maxima duracao do clique deve ser maior que a minima.\n");
        return 1;
    }

    printf("Selecione a variacao da duracao do clique (%%) / Recomendado: 10%%: ");
    scanf("%f", &config.variacaoClique);
    if(config.variacaoClique < 0) {
        printf("A variacao da duracao do clique deve ser maior que 0.\n");
        return 1;
    }

    const float CPS = 1000.0f / config.inputCPS;

    printf("Clique 'B' para alternar o clique dentro do inventario.\n");
    printf("Clique 'N' para alternar o clique dentro do inventario.\n");

    while (config.ativo) {
        // Hotkey para alterar funcionalidades
        if (GetAsyncKeyState('B') & 0x8000) {
            config.clicarInventario = !config.clicarInventario;
            printf("Inventario: %s\n", config.clicarInventario ? "ON" : "OFF");
            Sleep(200);
        }

        if (GetAsyncKeyState('N') & 0x8000) {
            config.quebrarBlocos = !config.quebrarBlocos;
            printf("Quebrar Blocos: %s\n", config.quebrarBlocos ? "ON" : "OFF");
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clicarInventario || !cursorVisivel()) {
                float duracaoClique = randomizar(config.durMinClique, config.durMaxClique);
                float delayVariacao = randomizar(-(config.variacaoClique / 100.0f), (config.variacaoClique / 100.0f)) * CPS;
                float cliqueRandomizado = CPS - duracaoClique + delayVariacao;

                if (cliqueRandomizado < 5) cliqueRandomizado = 5;

                // Simula o clique com delays previamente randomizados
                    enviarClique(true);
                    Sleep((int)duracaoClique);
                    if(!config.quebrarBlocos) {
                        enviarClique(false);
                    }
                    Sleep((int)cliqueRandomizado);
            }
        } else {
            Sleep(1); // Reduz o uso da CPU
        }
    }
    return 0;
}