#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <winuser.h>
#include <string.h>
#include <shellapi.h>

bool cursor_visible(void) {
    CURSORINFO ci;
    ci.cbSize = sizeof(CURSORINFO);
    
    if (!GetCursorInfo(&ci))
        return false;

    HCURSOR handle = ci.hCursor;
    if ((handle > (HCURSOR)50000) && (handle < (HCURSOR)100000))
        return true;

    return false;
}

// Simulate a left click at current cursor position
void SendRealClick() {
    INPUT inputs[2] = {0};
    
    // Mouse down
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    
    // Mouse up
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    
    SendInput(2, inputs, sizeof(INPUT));
}


int main() {
    int cps;
    bool inventario = false, quebrarBlocos = false;

    // Seletor de CPS
    printf("Selecione o número de cliques por segundo (CPS):\n");
    scanf("%d", &cps);

    if(cps < 1) {
        printf("CPS deve ser maior que 0.\n");
        return 0;
    }

    printf("Pressione a tecla 'B' clicar dentro do inventario.\n");

    while(true) {
        if (GetAsyncKeyState('B') & 0x8000 != 0) {
            inventario = !inventario;
            printf("Inventario: %s!\n", inventario ? "Ativado" : "Desativado");
        }

            if (inventario == true) {
                while (GetAsyncKeyState(VK_LBUTTON) & 0x8000 && !cursor_visible()) {
                    SendRealClick();
                    Sleep(1000 / cps);

                    Sleep(1); // Reduce CPU usage

                    if (GetAsyncKeyState('B') & 0x8000 != 0) {
                        inventario = !inventario;
                        printf("Inventario: %s!\n", inventario ? "Ativado" : "Desativado");
                    }
                }
            } else {
                while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    SendRealClick();
                    Sleep(1000 / cps);

                    Sleep(1); // Reduce CPU usage

                    if (GetAsyncKeyState('B') & 0x8000 != 0) {
                        inventario = !inventario;
                        printf("Inventario: %s!\n", inventario ? "Ativado" : "Desativado");
                    }
                }
            }
    }
}