#include "shell.h"
#include "fs.h"

extern void terminal_putchar(char c);
extern void terminal_writestring(const char* data);
extern void terminal_initialize(void);

static char cmd_buf[256];
static int cmd_len = 0;

/* 内部字符串工具 */
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static int strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) { ++s1; ++s2; --n; }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* 截断字符串右侧的空白字符（修复按键重复触发/延迟判定问题） */
static void rtrim(char* str) {
    int len = 0;
    while(str[len]) len++;
    while(len > 0 && str[len-1] == ' ') {
        str[len-1] = '\0';
        len--;
    }
}

/* 打印带当前路径的提示符 */
static void print_prompt(void) {
    char cwd[256];
    fs_get_cwd(cwd);
    terminal_writestring("root@minios:");
    terminal_writestring(cwd);
    terminal_writestring("# ");
}

void shell_init(void) {
    terminal_writestring("\n");
    print_prompt();
}

static void execute_command(void) {
    if (cmd_len == 0) return;
    cmd_buf[cmd_len] = '\0';
    rtrim(cmd_buf);
    
    if (cmd_buf[0] == '\0') return;

    if (strcmp(cmd_buf, "help") == 0) {
        terminal_writestring("Available Commands:\n");
        terminal_writestring("  help       - Show this message\n");
        terminal_writestring("  clear      - Clear the screen\n");
        terminal_writestring("  ls         - List files and directories\n");
        terminal_writestring("  mkdir <d>  - Create a directory\n");
        terminal_writestring("  cd <dir>   - Change directory\n");
        terminal_writestring("  pwd        - Print working directory\n");
        terminal_writestring("  format     - Format the disk\n");
        terminal_writestring("  cat <file> - Read a file\n");
        terminal_writestring("  write <f> <c> - Write a file\n");
        terminal_writestring("  exec <file>- Run a binary program\n");
    } 
    else if (strcmp(cmd_buf, "clear") == 0) {
        terminal_initialize();
    }
    else if (strcmp(cmd_buf, "ls") == 0) {
        fs_list();
    } 
    else if (strcmp(cmd_buf, "pwd") == 0) {
        fs_pwd();
    }
    else if (strcmp(cmd_buf, "format") == 0) {
        fs_format();
    } 
    else if (strncmp(cmd_buf, "mkdir ", 6) == 0) {
        fs_mkdir(cmd_buf + 6);
    }
    else if (strncmp(cmd_buf, "cd ", 3) == 0) {
        fs_cd(cmd_buf + 3);
    }
    else if (strncmp(cmd_buf, "cat ", 4) == 0) {
        fs_read_file(cmd_buf + 4);
    } 
    else if (strncmp(cmd_buf, "write ", 6) == 0) {
        char* name = cmd_buf + 6;
        char* content = name;
        while (*content && *content != ' ') content++;
        if (*content == ' ') {
            *content = '\0';
            content++;
            fs_write_file(name, content);
        } else {
            terminal_writestring("Usage: write <filename> <content>\n");
        }
    }
    else if (strncmp(cmd_buf, "exec ", 5) == 0) {
        extern void loader_exec(const char* filename);
        loader_exec(cmd_buf + 5);
    }
    else {
        terminal_writestring("Unknown command. Type 'help' for a list.\n");
    }
}

void shell_input(char c) {
    if (c == '\n') {
        terminal_putchar('\n');
        execute_command();
        cmd_len = 0;
        print_prompt();
    } else if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            terminal_putchar('\b');
        }
    } else {
        if (cmd_len < 255) {
            cmd_buf[cmd_len++] = c;
            terminal_putchar(c);
        }
    }
}