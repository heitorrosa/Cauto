#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <winuser.h>


int main() {

    int cps;
    bool status = false, inventario, quebrarBlocos;


    // Seletor de CPS
    printf("Selecione o número de cliques por segundo (CPS):\n");
    scanf("%d", &cps);

    if(cps < 1) {
        printf("CPS deve ser maior que 0.\n");
        return 0;
    }

    printf("Pressione a tecla 'V' iniciar o AutoClicker.\n");

    while(true) {
        if (GetAsyncKeyState('V') & 0x8000 != 0) {
            status = !status;
            printf("AutoClicker %s!\n", status ? "Ativado" : "Desativado");
        }
    }
}