#include "../../include/common.h"
#include "../../project/utils.c"

void getClickDuration();
void leftClickerHandler();

void leftClickerHandler() {
   sendPostMessageA(true);
   Sleep(100);
   sendPostMessageA(false);
}