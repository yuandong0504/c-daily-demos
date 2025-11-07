#include <stdio.h>

enum {
    ACT_OK   = 0,
    ACT_DROP = 1 << 0,  // bit0
    ACT_LOG  = 1 << 1,  // bit1
    ACT_ALERT= 1 << 2   // bit2
};

void print_bits(int v) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (v >> i) & 1);
    }
}

int main() {
    int action = 0;

    // 模拟打开多个标志
//    action |= ACT_DROP;
    action |= ACT_LOG;
//    action |= ACT_ALERT;
    
    printf("当前 action = ");
    print_bits(action);
    printf(" (十进制=%d)\n", action);

    // 检查每个 bit 是否打开
    if (action & ACT_DROP)
        printf("bit0: 丢包 ✅\n");
    else
        printf("bit0: 丢包 ❌\n");

    if (action & ACT_LOG)
        printf("bit1: 记录日志 ✅\n");
    else
        printf("bit1: 记录日志 ❌\n");

    if (action & ACT_ALERT)
        printf("bit2: 警报 ✅\n");
    else
        printf("bit2: 警报 ❌\n");


action = 0;
printf("初始状态: "); print_bits(action); puts("");

action |= ACT_LOG;
printf("打开日志: "); print_bits(action); puts("");

action |= ACT_DROP;
printf("再加丢包: "); print_bits(action); puts("");

action &= ~ACT_LOG;
printf("关闭日志: "); print_bits(action); puts("");
    return 0;

printf("test:5=%b\n",5);
}
