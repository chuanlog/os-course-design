.section .text

/* 
 * void gdt_flush(uint32_t gp)
 * 刷新 GDT 全局描述符表 
 */
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    
    /* 刷新数据段寄存器为 0x10 (内核数据段) */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* 远跳转以刷新代码段寄存器 CS 为 0x08 (内核代码段) */
    ljmp $0x08, $flush
flush:
    ret

/* 
 * void idt_load(uint32_t idtp)
 * 加载 IDT 指针 
 */
.global idt_load
.type idt_load, @function
idt_load:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

/* 
 * 键盘中断处理程序 (IRQ1 被重映射为 INT 33) 
 */
.global isr33
.type isr33, @function
isr33:
    /* 保存所有通用寄存器 */
    pusha
    
    /* 清理方向标志，防止 C 代码中的字符串操作出错 */
    cld
    
    /* 调用 C 语言编写的键盘处理函数 */
    call keyboard_handler
    
    /* 恢复寄存器 */
    popa
    
    /* 中断返回指令 */
    iret

/* 
 * 默认中断处理程序，捕获未处理的异常防止系统重启 (Triple Fault)
 */
.global isr_default
.type isr_default, @function
isr_default:
    pusha
    /* 发送 EOI 给 PIC，防止 PIC 挂起 */
    mov $0x20, %al
    out %al, $0x20
    out %al, $0xA0
    popa
    iret
