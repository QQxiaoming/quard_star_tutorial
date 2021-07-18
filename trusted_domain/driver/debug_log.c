#include <FreeRTOS.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "quard_star.h"
#include "ns16550.h"
#include "debug_log.h"

#define UART_LOG_BUFF_SIZE 1024

char *mystrcpy(char *dest, const char *src);
char *mystrcat(char *dest, const char *src);
unsigned int mystrlen(const char *s);
void *mymemset(void *s, int c, unsigned int count);
void myitoa(unsigned int n, char *buf);
int myatoi(char *pstr);
void myxtoa(unsigned int n, char *buf);
int myisDigit(unsigned char c);
int myisLetter(unsigned char c);
int _puts(char *str);

void *mymemcpy(void *dest, const void *src, unsigned int count)
{
    uint8_t * _src = (uint8_t *)src;
    uint8_t * _dst = (uint8_t *)dest;
    for(int i = count-1; i >= 0 ;i--)
    {
        *(_dst+i) = *(_src+i);
    }
    return dest;
}

/*
*功能：整型(int) 转化成 字符型(char)
*注意：不用 % / 符号的话，只能正确打印:0...9的数字对应的字符'0'...'9'
*/
void myitoa(unsigned int n, char *buf)
{
    int i;

    if (n < 10)
    {
        buf[0] = n + '0';
        buf[1] = '\0';
        return;
    }
    myitoa(n / 10, buf);

    for (i = 0; buf[i] != '\0'; i++)
        ;

    buf[i] = (n % 10) + '0';

    buf[i + 1] = '\0';
}

/*
*功能：字符型(char) 转化成 整型(int)
*/
int myatoi(char *pstr)
{
    int int_ret = 0;
    int int_sign = 1; //正负号标示 1:正数 -1:负数

    if (*pstr == '\0') //判断指针是否为空
    {
        return -1;
    }
    while (((*pstr) == ' ') || ((*pstr) == '\n') || ((*pstr) == '\t') || ((*pstr) == '\b'))
    {
        pstr++; //跳过前面的空格字符
    }

    /*
        * 判断正负号
        * 如果是正号，指针指向下一个字符
        * 如果是符号，把符号标记为Integer_sign置-1，然后再把指针指向下一个字符
        */
    if (*pstr == '-')
    {
        int_sign = -1;
    }
    if (*pstr == '-' || *pstr == '+')
    {
        pstr++;
    }

    while (*pstr >= '0' && *pstr <= '9') //把数字字符串逐个转换成整数，并把最后转换好的整数赋给Ret_Integer
    {
        int_ret = int_ret * 10 + *pstr - '0';
        pstr++;
    }
    int_ret = int_sign * int_ret;

    return int_ret;
}

/*
*功能：16进制字(0x) 转化成 字符型(char)
*注意：不用 % / 符号的话，只能正确打印，0...9..15的数字,对应的'0'...'9''A'...'F'
*注意：由于编译问题，这个函数，暂时由uart_sendByte_hex()函数替代
*/
void myxtoa(unsigned int n, char *buf)
{
    int i;
    if (n < 16)
    {
        if (n < 10)
        {
            buf[0] = n + '0';
        }
        else
        {
            buf[0] = n - 10 + 'a';
        }
        buf[1] = '\0';
        return;
    }
    myxtoa(n / 16, buf);

    for (i = 0; buf[i] != '\0'; i++)
        ;

    if ((n % 16) < 10)
    {
        buf[i] = (n % 16) + '0';
    }
    else
    {
        buf[i] = (n % 16) - 10 + 'a';
    }
    buf[i + 1] = '\0';
}

/*
 * 判断一个字符是否数字
 */
int myisDigit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;
}

/*
 * 判断一个字符是否英文字母
 */
int myisLetter(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    else if (c >= 'A' && c <= 'Z')
        return 1;
    else
        return 0;
}

/**
 * mymemset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use mymemset() to access IO space, use memset_io() instead.
 */
void *mymemset(void *s, int c, unsigned int count)
{
    char *xs = (char *)s;

    while (count--)
        *xs++ = c;

    return s;
}

/**
 * mystrcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
char *mystrcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

/**
 * mystrlen - Find the length of a string
 * @s: The string to be sized
 */
unsigned int mystrlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

/**
 * mystrcat - Append one %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 */
char *mystrcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest)
        dest++;
    while ((*dest++ = *src++) != '\0')
        ;

    return tmp;
}

static char printk_string[UART_LOG_BUFF_SIZE] = {0};

/*功能：格式化打印一个字符串
*参数：格式化的字符串
*注意：这个是简易版本 (%02x 完成)
* %-3s不行， %f也不行， %X不行
*/
int _printf(char *fmt, ...)
{
    char *str = printk_string;
    int count = 0;
    char c;
    char *s;
    int n;

    int index = 0;
    int ret = 2;

    char buf[65];
    char digit[16];
    int num = 0;
    int len = 0;

    mymemset(printk_string, 0, UART_LOG_BUFF_SIZE);
    mymemset(buf, 0, sizeof(buf));
    mymemset(digit, 0, sizeof(digit));

    va_list ap;

    va_start(ap, fmt);

    while (*fmt != '\0')
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
            case 'd': /*整型*/
            {
                n = va_arg(ap, int);
                if (n < 0)
                {
                    *str = '-';
                    str++;
                    n = -n;
                }
                myitoa(n, buf);
                mymemcpy(str, buf, mystrlen(buf));
                str += mystrlen(buf);
                break;
            }
            case 'c': /*字符型*/
            {
                c = va_arg(ap, int);
                *str = c;
                str++;

                break;
            }
            case 'x': /*16进制*/
            {
                n = va_arg(ap, int);
                myxtoa(n, buf);
                mymemcpy(str, buf, mystrlen(buf));
                str += mystrlen(buf);
                break;
            }
            case 's': /*字符串*/
            {
                s = va_arg(ap, char *);
                mymemcpy(str, s, mystrlen(s));
                str += mystrlen(s);
                break;
            }
            case '%': /*输出%*/
            {
                *str = '%';
                str++;

                break;
            }
            case '0': /*位不足的左补0*/
            {
                index = 0;
                num = 0;
                mymemset(digit, 0, sizeof(digit));

                while (1)
                {
                    fmt++;
                    ret = myisDigit(*fmt);
                    if (ret == 1) //是数字
                    {
                        digit[index] = *fmt;
                        index++;
                    }
                    else
                    {
                        num = myatoi(digit);
                        break;
                    }
                }
                switch (*fmt)
                {
                case 'd': /*整型*/
                {
                    n = va_arg(ap, int);
                    if (n < 0)
                    {
                        *str = '-';
                        str++;
                        n = -n;
                    }
                    myitoa(n, buf);
                    len = mystrlen(buf);
                    if (len >= num)
                    {
                        mymemcpy(str, buf, mystrlen(buf));
                        str += mystrlen(buf);
                    }
                    else
                    {
                        mymemset(str, '0', num - len);
                        str += num - len;
                        mymemcpy(str, buf, mystrlen(buf));
                        str += mystrlen(buf);
                    }
                    break;
                }
                case 'x': /*16进制*/
                {
                    n = va_arg(ap, int);
                    myxtoa(n, buf);
                    len = mystrlen(buf);
                    if (len >= num)
                    {
                        mymemcpy(str, buf, len);
                        str += len;
                    }
                    else
                    {
                        mymemset(str, '0', num - len);
                        str += num - len;
                        mymemcpy(str, buf, len);
                        str += len;
                    }
                    break;
                }
                case 's': /*字符串*/
                {
                    s = va_arg(ap, char *);
                    len = mystrlen(s);
                    if (len >= num)
                    {
                        mymemcpy(str, s, mystrlen(s));
                        str += mystrlen(s);
                    }
                    else
                    {
                        mymemset(str, '0', num - len);
                        str += num - len;
                        mymemcpy(str, s, mystrlen(s));
                        str += mystrlen(s);
                    }
                    break;
                }
                default:
                    break;
                }
            }
            default:
                break;
            }
        }
        else
        {
            *str = *fmt;
            str++;

            if (*fmt == '\n')
            {
            }
        }
        fmt++;
    }

    va_end(ap);
    
    
    _puts(printk_string);

    return count;
}

int _puts(char *str)
{
    int counter = 0;
    if (!str)
    {
        return 0;
    }
    while (*str && (counter < UART_LOG_BUFF_SIZE))
    {
        if(*str == '\n')
	        vOutNS16550( NS16550_ADDR, '\r' );
	    vOutNS16550( NS16550_ADDR, *str++ );
        counter++;
    }
    return counter;
}


