#include "model.h"
int main(void){
    File f = (File){1, 1024, "demo"};
    (void)f;  // 防止未使用告警
    return 0;
}
