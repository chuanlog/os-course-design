/*
 * MiniOS C application runtime entry.
 *
 * The loader jumps to byte 0 of the flat binary. For C apps, helper functions
 * may be placed before app_main, so this small assembly file must be linked
 * first to guarantee that byte 0 is always the real entry code.
 */
.section .text
.global _start
.extern app_main

_start:
    push %ebp
    mov %esp, %ebp

    /* Forward loader-provided arguments: app_main(print, getchar). */
    push 12(%ebp)
    push 8(%ebp)
    call app_main
    add $8, %esp

    pop %ebp
    ret
