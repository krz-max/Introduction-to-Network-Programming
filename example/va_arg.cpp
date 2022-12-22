#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int func1(int a, ...) {
    va_list ap;
    va_start(ap, a);
    int in = va_arg(ap, int);
    printf("%d\n", in);
    in = va_arg(ap, int);
    printf("%d\n", in);
    va_end(ap);
}

int func2(const char *fmt, ...) {
    va_list ap;
    char* str;
    int argv;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    str = va_arg(ap, char*);
    argv = va_arg(ap, int);
    printf("%s, %d\n", str, argv);
    va_end(ap);
}
void foo(char *fmt, ...)
{
    va_list ap;
    int d;
    char c, *s;

    va_start(ap, fmt);
    while (*fmt)
        switch (*fmt++) {
        case 's':              /* string */
            s = va_arg(ap, char *);
            printf("string %s\n", s);
            break;
        case 'd':              /* int */
            d = va_arg(ap, int);
            printf("int %d\n", d);
            break;
        case 'c':              /* char */
            /* need a cast here since va_arg only
               takes fully promoted types */
            c = (char) va_arg(ap, int);
            printf("char %c\n", c);
            break;
        }
    va_end(ap);
}
int main() {
    char s[] = {"dcs"};
    func1(10, 20, 30, 40, 50);
    func2("What do you mean?\n", "20", 30);
    foo(s, 10, 'd', "Hello, World\n");
    return 0;
}