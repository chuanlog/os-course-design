#include "io.h"
#include <stdint.h>

extern void terminal_putchar(char c);
extern void shell_input(char c);

/* 一个极其简化的美式 QWERTY 键盘扫描码映射表 */
const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  ' ', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 键盘中断处理函数，当你在键盘上按键时会被触发 */
void keyboard_handler(void) {
    /* 0x60 是键盘的数据端口，读取它获取扫描码 */
    uint8_t scancode = inb(0x60);
    
    /* 如果最高位是 1，代表按键松开 (Key Release) */
    if (scancode & 0x80) {
        // 当前阶段我们忽略按键松开事件
    } else {
        /* 按键按下 (Key Press)，将其转换为 ASCII 字符 */
        char c = kbd_us[scancode];
        if (c) {
            /* 将字符发送到 Shell 缓冲区处理 */
            shell_input(c);
        }
    }
    
    /* 发送 EOI (End of Interrupt) 信号给主 PIC (端口 0x20)
     * 只有发送了这个信号，PIC 才会继续发送下一个中断
     */
    outb(0x20, 0x20);
}
