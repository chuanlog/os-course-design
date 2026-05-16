/* calc.c - 一个运行在 MiniOS 上的 C 语言独立应用程序！ */

/* 内核传给我们的 API 函数类型定义 */
typedef void (*print_t)(const char*);
typedef char (*getchar_t)(void);

/* 工具函数：打印数字 */
static void print_num(print_t print, int num) {
    if (num == 0) { 
        print("0"); 
        return; 
    }
    if (num < 0) { 
        print("-"); 
        num = -num; 
    }
    char buf[16];
    int i = 14;
    buf[15] = '\0';
    while (num > 0) {
        buf[i--] = (num % 10) + '0';
        num /= 10;
    }
    print(&buf[i+1]);
}

/* 
 * 程序的真正入口点：必须是文件中的第一个函数！
 * 链接器会将它放在文件的开头。当 loader 跳转时，就会执行这里。
 * 内核在调用时传递了两个参数：打印函数和读取按键函数。
 */
void app_main(print_t print, getchar_t getchar) {
    print("\n===============================\n");
    print("  MiniOS Interactive Calculator\n");
    print("===============================\n\n");
    
    while (1) {
        print("Enter first number (or 'q' to quit): ");
        int a = 0;
        char c;
        int is_quit = 0;
        
        while ((c = getchar()) != '\n') {
            if (c == 'q') {
                is_quit = 1;
                char s[2] = {c, 0};
                print(s);
            } else if (c >= '0' && c <= '9') {
                a = a * 10 + (c - '0');
                char s[2] = {c, 0};
                print(s);
            }
        }
        print("\n");
        
        if (is_quit) {
            print("Goodbye!\n");
            break;
        }
        
        print("Enter operation (+, -, *, /): ");
        char op = 0;
        while ((c = getchar()) != '\n') {
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                op = c;
                char s[2] = {c, 0};
                print(s);
                break; /* 获取到符号就跳出循环 */
            }
        }
        /* 等待用户按下回车确认符号 */
        while (getchar() != '\n');
        print("\n");
        
        print("Enter second number: ");
        int b = 0;
        while ((c = getchar()) != '\n') {
            if (c >= '0' && c <= '9') {
                b = b * 10 + (c - '0');
                char s[2] = {c, 0};
                print(s);
            }
        }
        print("\n");
        
        print("Result: ");
        print_num(print, a);
        char s[4] = {' ', op, ' ', 0};
        print(s);
        print_num(print, b);
        print(" = ");
        
        if (op == '+') print_num(print, a + b);
        else if (op == '-') print_num(print, a - b);
        else if (op == '*') print_num(print, a * b);
        else if (op == '/') {
            if (b == 0) print("Error: Divide by zero!");
            else print_num(print, a / b);
        }
        print("\n\n");
    }
}
