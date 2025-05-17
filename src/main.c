#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "utils.c"

int main() {
    
    // Configura o Autoclicker com os valores padrão
    configCauto config;
    init_config(&config);

    printf("CPS desejado: ");
    scanf("%d", &config.inputCPS);

    if(config.inputCPS < 1) {
        printf("O CPS deve ser maior que 0.\n");
        return 1;
    }

    fflush(stdin); // Limpa o buffer de entrada

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

    while (true) {
        if (GetAsyncKeyState('B') & 0x8000) {
            config.clicarInventario = !config.clicarInventario;
            printf("Inventario: %s\n", config.clicarInventario ? "ON" : "OFF");
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (config.clicarInventario || !cursorVisivel()) {

                // Randomiza o delay entre os cliques
                float duracaoClique = randomizar(config.durMinClique, config.durMaxClique); // 22-30ms duracao do clique

                // Randomiza a quantia de Cliques (Persistence)
                float delayVariacao = randomizar(-(config.variacaoClique / 100.0f), (config.variacaoClique / 100.0f)) * CPS;

                // Randomiza a frequencia de cliques (Spike e Drop)
                // float randSpike = randomizar(-(randSpike / 100.0f)) * CPS; // Drop
                // float randDrop = randomizar((randDrop / 100.0f)) * CPS; // Spike

                // Randomizador completo
                float cliqueRandomizado = CPS - duracaoClique + delayVariacao; // (delaySpike + delayDrop)

                // delay minimo
                if (cliqueRandomizado < 5) cliqueRandomizado = 5;

                // Executa a funcao do clique
                enviarClique(true);
                Sleep((int)duracaoClique);
                enviarClique(false);
                Sleep((int)cliqueRandomizado);
            }
        }
        else {
            Sleep(1);
        }
    }
    return 0;
}