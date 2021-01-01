#include<stdio.h>
#include<stdlib.h>
#include <stdarg.h>
#include "myprintf.h"
#include<stdarg.h>

char *ctable = "0123456789ABCDEF";

int rpu(u32 x, int base)
{
    char c; 
    if(x) {
        c = ctable[x % base];
        rpu(x / base, base);
        putchar(c);
    }
}

int printu(u32 x, int base)
{
    (x==0) ? putchar('0') : rpu(x, base);
    putchar(' ');
}

void prints(char *str)
{
    for(int i = 0; str[i] != '\0'; i++) 
    {
        putchar(str[i]);
    }
}

int printd(int x)
{
    if(x<0)
    {
        putchar('-');
        x = x * -1;
    }
    printu(x, 10);
    
}

int printx(u32 x)
{
    prints("0X");
    printu(x, 16);
}

int printo(u32 x)
{
    prints("o");
    printu(x,8);
}

void myprintf(char *fmt, ...)
{
    va_list vlist; //creating a va_list type
    va_start(vlist, fmt); // initializing the va list with passed parameters

    while(*(fmt) != 0)
    {
        if( *fmt == '%')
        {
            fmt++;
            if(*fmt == 's')
            {
                prints(va_arg(vlist, char *));
            }
            else if(*fmt == 'd')
            {
                printd(va_arg(vlist, int));
            }
            else if(*fmt == 'c')
            {
                putchar(va_arg(vlist, int));
            }
            else if(*fmt == 'u')
            {
                printu(va_arg(vlist, u32), 10);
            }
            else if(*fmt == 'x')
            {
                printx(va_arg(vlist, u32));
            }
            else if(*fmt == 'o')
            {
                printo(va_arg(vlist, u32));
            }
            else {
                return;
            }
            
        }
        else if(*fmt == '\n')
        {
            prints("\r\n");
        }
        else
        {
            putchar(*fmt);
        }
        fmt++;
    }
    va_end(vlist); // to clean up
}

int main(int argc, char *argv[ ], char *env[ ])
{

    myprintf("cha=%c string=%s dec=%d hex=%x oct=%o neg=%d\n", 
	   'A', "this is a test", 100,    100,   100,  -100);


    printf("Print argc and argv: \n");
    printf("Value of argc: %d\n", argc);
    for(int i = 0; i < argc; i ++)
    {
      printf("value of argv[%d]: %s\n", i, argv[i]);
    }

}