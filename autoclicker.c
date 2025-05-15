#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

// Randomizador de cliques
float randomizar(float min, float max) {
    static unsigned int seed = 0;
    if (!seed) seed = GetTickCount() ^ GetCurrentProcessId();
    seed = (214013 * seed + 2531011);
    return min + (seed >> 16) * (1.0f / 65535.0f) * (max - min);
}

bool cursorVisivel(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;

    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

void enviarClique(bool down) {
    POINT pos;
    GetCursorPos(&pos);
    HWND window = GetForegroundWindow();
    PostMessage(window, down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
}

int main() {
    int CPS;
    float duracaoMinimaClique, duracaoMaximaClique, VariacaoClique;
    printf("CPS desejado: ");
    scanf("%d", &CPS);

    if(CPS < 1) {
        printf("O CPS deve ser maior que 0.\n");
        return 1;
    }

    fflush(stdin); // Limpa o buffer de entrada

    printf("Selecione a minima duracao do clique (ms) / Recomendado: 22ms: ");
    scanf("%f", &duracaoMinimaClique);
    if(duracaoMinimaClique < 10) {
        printf("A duracao do clique deve ser maior que 10.\n");
        return 1;
    }

    printf("Selecione a maxima duracao do clique (ms) / Recomendado: 30ms: ");
    scanf("%f", &duracaoMaximaClique);
    if(duracaoMaximaClique < duracaoMinimaClique) {
        printf("A maxima duracao do clique deve ser maior que a minima.\n");
        return 1;
    }

    printf("Selecione a variacao da duracao do clique (%%) / Recomendado: 10%%: ");
    scanf("%f", &VariacaoClique);
    if(VariacaoClique < 0) {
        printf("A variacao da duracao do clique deve ser maior que 0.\n");
        return 1;
    }

    const float delayBase = 1000.0f / CPS;
    bool inventario = false;

    printf("Clique 'B' para alternar o clique dentro do inventario.\n");

    while (true) {
        if (GetAsyncKeyState('B') & 0x8000) {
            inventario = !inventario;
            printf("Inventario: %s\n", inventario ? "ON" : "OFF");
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (inventario || !cursorVisivel()) {

                // Randomizador para os cliques (Ajuda no Bypass de Anti-Cheats) //

                // Randomiza o delay entre os cliques
                float duracaoClique = randomizar(duracaoMinimaClique, duracaoMaximaClique); // 22-30ms duracao do clique

                // Randomiza a quantia de Cliques (Persistence)
                float delayVariacao = randomizar(-(VariacaoClique / 100.0f), (VariacaoClique / 100.0f)) * delayBase; // ±10% variacao do delay

                float randomizador = delayBase - duracaoClique + delayVariacao;

                // delay minimo
                if (randomizador < 5) randomizador = 5;

                // Executa a funcao do clique
                enviarClique(true);
                Sleep((int)duracaoClique);
                enviarClique(false);
                Sleep((int)randomizador);
            }
        }
        else {
            Sleep(1);
        }
    }
    return 0;
}