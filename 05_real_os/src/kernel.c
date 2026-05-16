#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* VGA 文本模式颜色 */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_LIGHT_GREEN = 10,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

#include "io.h"

/* 启用并更新 VGA 硬件光标 */
void update_cursor(int x, int y) {
    uint16_t pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            const size_t index = y * 80 + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    /* 启用光标，设置光标形状为下划线，并初始化位置 */
    enable_cursor(14, 15);
    update_cursor(0, 0);
}

/* 终端屏幕向上滚动一行 */
static void terminal_scroll(void) {
    /* 将第 1~24 行的数据整体向上平移一行到 0~23 行 */
    for (size_t y = 1; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            terminal_buffer[(y - 1) * 80 + x] = terminal_buffer[y * 80 + x];
        }
    }
    /* 将最后一行 (第 24 行) 清空为黑底空格 */
    for (size_t x = 0; x < 80; x++) {
        terminal_buffer[24 * 80 + x] = vga_entry(' ', terminal_color);
    }
}

/* 支持退格键和换行符的字符打印 */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == 25) {
            terminal_scroll();
            terminal_row = 24;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = 79;
        }
        /* 清除退格位置的字符 */
        const size_t index = terminal_row * 80 + terminal_column;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
        update_cursor(terminal_column, terminal_row);
        return;
    }

    const size_t index = terminal_row * 80 + terminal_column;
    terminal_buffer[index] = vga_entry(c, terminal_color);
    if (++terminal_column == 80) {
        terminal_column = 0;
        if (++terminal_row == 25) {
            terminal_scroll();
            terminal_row = 24;
        }
    }
    update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

/* 引入在其他文件中定义的函数 */
extern void gdt_init(void);
extern void idt_init(void);
extern void pic_remap(void);
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void isr33(void); /* 定义在 interrupt.S 中 */

#include "multiboot.h"
#include "pmm.h"

/* 打印十进制数字 */
void terminal_writedec(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    char buf[11];
    int i = 9;
    buf[10] = '\0';
    while (num > 0) {
        buf[i] = (num % 10) + '0';
        num /= 10;
        i--;
    }
    terminal_writestring(&buf[i+1]);
}

/* 打印十六进制地址 */
void terminal_writehex(uint32_t num) {
    terminal_writestring("0x");
    if (num == 0) {
        terminal_writestring("00000000");
        return;
    }
    char buf[9];
    buf[8] = '\0';
    const char* hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        buf[i] = hex[num & 0xF];
        num >>= 4;
    }
    terminal_writestring(buf);
}

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
    terminal_initialize();
    
    terminal_writestring("===================================================\n");
    terminal_writestring("                Welcome to MiniOS!                 \n");
    terminal_writestring("===================================================\n\n");
    
    terminal_writestring("[INFO] Initializing GDT...\n");
    gdt_init();
    
    terminal_writestring("[INFO] Initializing IDT and PIC...\n");
    idt_init();
    pic_remap();
    
    /* 注册键盘中断处理程序到 INT 33 (IRQ 1) */
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    
    terminal_writestring("[INFO] Enabling interrupts...\n");
    /* 开启 CPU 中断接收 (sti 指令) */
    __asm__ volatile("sti");
    
    /* --- Milestone 3: 初始化物理内存分配器 --- */
    if (magic == 0x2BADB002) {
        terminal_writestring("[INFO] Multiboot magic is valid.\n");
        terminal_writestring("[INFO] Available Upper Memory: ");
        terminal_writedec(mbi->mem_upper / 1024);
        terminal_writestring(" MB\n");
        
        terminal_writestring("[INFO] Initializing Physical Memory Manager (PMM)...\n");
        pmm_init(mbi->mem_upper);
        
        terminal_writestring("       Free pages available: ");
        terminal_writedec(pmm_get_free_pages());
        terminal_writestring("\n");
        
        /* 测试内存分配 */
        terminal_writestring("[TEST] Allocating 3 memory pages...\n");
        void* page1 = pmm_alloc_page();
        void* page2 = pmm_alloc_page();
        void* page3 = pmm_alloc_page();
        
        terminal_writestring("       Page 1: "); terminal_writehex((uint32_t)page1); terminal_writestring("\n");
        terminal_writestring("       Page 2: "); terminal_writehex((uint32_t)page2); terminal_writestring("\n");
        terminal_writestring("       Page 3: "); terminal_writehex((uint32_t)page3); terminal_writestring("\n");
        
        terminal_writestring("[TEST] Freeing Page 2...\n");
        pmm_free_page(page2);
        
        void* page4 = pmm_alloc_page();
        terminal_writestring("       Allocated new Page: "); terminal_writehex((uint32_t)page4); terminal_writestring(" (Should be same as Page 2)\n");
    } else {
        terminal_writestring("[ERROR] Invalid Multiboot magic number!\n");
    }

    terminal_writestring("\n[INFO] Initializing Disk and File System...\n");
    extern void fs_init(void);
    fs_init();

    terminal_writestring("\n[SUCCESS] Kernel booted successfully!\n");
    
    extern void shell_init(void);
    shell_init();
    
    /* 让 CPU 进入死循环，等待中断发生 */
    while (1) {
        __asm__ volatile("hlt");
    }
}
