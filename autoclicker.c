#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <winuser.h>
#include <string.h>
#include <shellapi.h>

// Define enums for mouse actions
typedef enum {
    LEFT_DOWN,
    LEFT_UP,
    RIGHT_DOWN,
    RIGHT_UP
} mouse_type_t;

typedef enum {
    LEFT,
    RIGHT
} mouse_side_t;

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

bool botaoEsquerdoPressionado(void) {
    return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
}

void send_input(mouse_type_t m_type, mouse_side_t m_side) {
    POINT pos;
    if (!GetCursorPos(&pos)) 
        return;

    HWND curr_wnd = GetForegroundWindow();
    
    // Convert our enum types to Windows message types
    UINT msg;
    switch (m_type) {
        case LEFT_DOWN: msg = WM_LBUTTONDOWN; break;
        case LEFT_UP: msg = WM_LBUTTONUP; break;
        case RIGHT_DOWN: msg = WM_RBUTTONDOWN; break;
        case RIGHT_UP: msg = WM_RBUTTONUP; break;
    }
    
    WPARAM wparam;
    switch (m_side) {
        case LEFT: wparam = MK_LBUTTON; break;
        case RIGHT: wparam = MK_RBUTTON; break;
    }
    
    PostMessage(curr_wnd, msg, wparam, MAKELPARAM(pos.x, pos.y));
}

void click(bool is_down, bool is_left) {
    if (is_down) {
        if (is_left) {
            send_input(LEFT_DOWN, LEFT);
        } else {
            send_input(RIGHT_DOWN, RIGHT);
        }
    } else {
        if (is_left) {
            send_input(LEFT_UP, LEFT);
        } else {
            send_input(RIGHT_UP, RIGHT);
        }
    }
}

int main() {
    int cps;
    bool inventario = false;

    // Seletor de CPS
    printf("Selecione o número de cliques por segundo (CPS):\n");
    scanf("%d", &cps);

    if(cps < 1) {
        printf("CPS deve ser maior que 0.\n");
        return 0;
    }

    printf("Pressione a tecla 'B' para clicar dentro do inventario.\n");

    while(true) {
        // Alternar entre os modos de Inventário
        if (GetAsyncKeyState('B') & 0x8000) {
            inventario = !inventario;
            printf("Inventario: %s!\n", inventario ? "Ativado" : "Desativado");
            Sleep(200);
        }

        // Lógica do Autoclicker
        if (botaoEsquerdoPressionado()) {
            if (inventario || !cursor_visible()) {
                    click(true, true); // down
                    Sleep(20);
                    click(false, true); // up
                }
                Sleep(1000 / cps);
            }
        }

        Sleep(1);

}
