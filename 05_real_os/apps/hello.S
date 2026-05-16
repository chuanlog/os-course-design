/* 
 * 外部应用程序：hello.S
 */
.section .text
.global _start

_start:
    /* 我们处于内核态，所以可以直接操作 VGA 显存来证明我们运行了！ */
    /* VGA 显存起始地址是 0xB8000 */
    mov $0xB8000, %edi
    
    /* 打印一个闪烁的红色警告条："[USER APP]" */
    movw $0xCE55, 0(%edi)  # U
    movw $0xCE53, 2(%edi)  # S
    movw $0xCE45, 4(%edi)  # E
    movw $0xCE52, 6(%edi)  # R
    movw $0xCE20, 8(%edi)  # (space)
    movw $0xCE41, 10(%edi) # A
    movw $0xCE50, 12(%edi) # P
    movw $0xCE50, 14(%edi) # P
    
    /* 制造一点延迟，让你能看清 */
    mov $0x0FFFFFFF, %ecx
delay:
    dec %ecx
    jnz delay

    /* 执行完毕，返回到内核 */
    ret