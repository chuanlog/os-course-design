/* 
 * Multiboot 规范的头部定义，告诉 GRUB 这是一个可以引导的内核 
 */
.set ALIGN,    1<<0             /* 将加载的模块按页边界对齐 */
.set MEMINFO,  1<<1             /* 让 GRUB 提供物理内存映射信息 */
.set FLAGS,    ALIGN | MEMINFO  /* 标志位 */
.set MAGIC,    0x1BADB002       /* 魔数：让引导程序找到这个头部 */
.set CHECKSUM, -(MAGIC + FLAGS) /* 校验和，证明我们是合法的 multiboot 格式 */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* 
 * 为内核预留一个栈空间 (16 KiB)
 */
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/* 
 * 内核执行的入口点 
 */
.section .text
.global _start
.type _start, @function
_start:
    /* 初始化栈指针 */
    mov $stack_top, %esp

    /* 将 Multiboot 信息压入栈，作为 kernel_main 的参数 */
    push %ebx  /* Multiboot 信息结构的指针 */
    push %eax  /* Multiboot 魔数 (0x2BADB002) */

    /* 跳转到 C 语言编写的 kernel_main 函数 */
    call kernel_main

    /* 如果 kernel_main 返回，则让 CPU 进入死循环，挂起系统 */
    cli
1:  hlt
    jmp 1b

/* 设置 _start 函数的大小 */
.size _start, . - _start
