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

/* Shift 状态下的美式 QWERTY 映射表 */
const char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
  '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 给外部应用使用的全局变量 */
volatile int app_running = 0;
volatile char kb_buffer = 0;
static uint8_t shift_pressed = 0;

/* 供外部应用调用的阻塞式读取字符函数 */
char kbd_getchar(void) {
    kb_buffer = 0;
    while (kb_buffer == 0) {
        /* 使用 hlt 指令让 CPU 挂起，等待下一个中断（键盘中断）唤醒，节省 CPU 资源 */
        __asm__ volatile("hlt");
    }
    char c = kb_buffer;
    kb_buffer = 0;
    return c;
}

/* 键盘中断处理函数，当你在键盘上按键时会被触发 */
void keyboard_handler(void) {
    /* 0x60 是键盘的数据端口，读取它获取扫描码 */
    uint8_t scancode = inb(0x60);
    char c = 0;

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }
    
    /* 如果最高位是 1，代表按键松开 (Key Release) */
    if (scancode & 0x80) {
        // 当前阶段我们忽略按键松开事件
    } else {
        /* 按键按下 (Key Press)，将其转换为 ASCII 字符 */
        c = shift_pressed ? kbd_us_shift[scancode] : kbd_us[scancode];
    }
    
    /* 发送 EOI (End of Interrupt) 信号给主 PIC (端口 0x20)
     * 只有发送了这个信号，PIC 才会继续发送下一个中断
     */
    outb(0x20, 0x20);

    if (!c) {
        return;
    }

    if (app_running) {
        kb_buffer = c;
        return;
    }

    /* 
     * shell_input() 可能执行 exec 等长时间运行的命令。
     * 如果仍停留在 IRQ 禁中断状态，应用里的 kbd_getchar()->hlt 将永远等不到下一次键盘中断。
     * 因此这里先完成 EOI，再临时开启中断，让应用运行期间可以继续接收键盘 IRQ。
     */
    __asm__ volatile("sti");
    shell_input(c);
    __asm__ volatile("cli");
}
