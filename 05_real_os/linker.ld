/* 内核入口点，我们在 boot.S 中定义了 _start */
ENTRY(_start)

SECTIONS
{
    /* 内核将被 GRUB 加载到物理地址 1MB 的位置（传统内核加载地址） */
    . = 1M;

    /* 必须首先放置 multiboot 头部，否则 GRUB 无法识别这是一个内核 */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    /* 只读数据 */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* 初始化的可读写数据 */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* 未初始化的可读写数据和栈 */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* 记录内核结束的物理地址，物理内存分配器将从这里开始管理空闲内存 */
    _kernel_end = .;
}
