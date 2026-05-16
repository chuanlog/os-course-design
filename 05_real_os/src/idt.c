#include "idt.h"
#include "io.h"

/* IDT 条目结构体 */
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

/* IDT 指针结构体，用于 lidt 指令 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

/* 该函数在 interrupt.S 中用汇编实现 */
extern void idt_load(uint32_t);
extern void isr_default(void);

/* 设置 IDT 门 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

/* 初始化 IDT */
void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    /* 将所有 IDT 表项都先注册为一个默认的空处理函数，防止未处理的中断（如时钟中断、页错误等）导致系统崩溃重启 */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_default, 0x08, 0x8E);
    }

    /* 加载 IDT */
    idt_load((uint32_t)&idtp);
}

/* 重新映射 PIC (可编程中断控制器)
 * 默认情况下，IRQ 0-7 映射到中断 8-15，这与 CPU 异常冲突。
 * 我们需要将它们映射到 32-47。
 */
void pic_remap(void) {
    /* 开启初始化模式 */
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    /* 重新映射偏移量：主 PIC 映射到 0x20 (32)，从 PIC 映射到 0x28 (40) */
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();

    /* 级联设置 */
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    /* 环境设置 (8086 模式) */
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    /* 屏蔽所有中断，除了 IRQ1 (键盘，0x02) */
    outb(0x21, ~(1 << 1));
    outb(0xA1, 0xFF);
}
