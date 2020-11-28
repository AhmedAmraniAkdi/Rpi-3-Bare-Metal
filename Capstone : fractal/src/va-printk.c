/*
   Should probably check that uart is enabled.
       The  functions  snprintf()  and vsnprintf() do not write more than size
       bytes (including the terminating null byte ('\0')).  If the output  was
       truncated  due  to  this  limit  then the return value is the number of
       characters (excluding the terminating null byte) which would have  been
       written  to the final string if enough space had been available.  Thus,
       a return value of size or more means that  the  output  was  truncated.
       (See also below under NOTES.)
*/
#include "assert.h"
#include "rpi.h"
#include <stdint.h>



void rpi_set_output(int (*putc_fp)(int), int (*puts_fp)(const char *)) {
    assert(putc_fp);
    rpi_putchar = putc_fp;
    if(puts_fp)
        putk = puts_fp;
}

// XXX: check sign promotion.
static int isdigit(int c) { return c >= '0' && c <= '9'; }


// don't move: can't be inlined, gcc seems to have a bug.
char * __emit_float(char *num, double d, unsigned wdith);

static char* pad(char *p,  char *base, int width) {
	int len = p - base;
	// pad with leading spaces
	for(; len < width; len++)
		*p++ = ' ';
	*p = 0;
	return p;
}

static char* reverse(char *dst, int n, char *p, char *start) {
	// drop the lower chars if the string is too big.
	int len = p - start + 1; // include \0
	if(len > n)
		start += (len - n);

	char *s = dst;
	
	// p starts at end.
	for(p--; p >= start; p--)
		*dst++ = *p;
	*dst++ = 0;

	return s;
}

static char* emit(unsigned base, char *dst, int n, uint64_t val, int width, int signed_p) {
	// XXX: constrain width to be >= bit size: can change this.
	if(width > n)
		width = n - 1;

	char buf[64], *p = buf; 
	uint64_t u = val;

	switch(base) { 
		case 10: 
		{
			unsigned neg_p = 0;
			if(val < 0 && signed_p) {
				neg_p = 1;
				val = -val;
				u = val;
			}
			do { 
				*p++ = "0123456789"[u % 10];
			} while(u /= 10); 
			if(neg_p)
				*p++ = '-';
			break;
		}
		case 16:
			u = val;
                        do {
                                *p++ = "0123456789abcdef"[u % 16];
                        } while(u /= 16);
                        break;
		// just a single char
		case 8:
			*p++ = val;
			break;
		case 2:
			u = val;
#if 0
			for(int i = 0; i < 32; i++)
				*p++ = "01"[(u & (1<<i)) != 0];
#else
                        do {
                        	*p++ = "01"[u % 2];
			} while(u /= 2);
#endif
			break;
		default:
			assert(0);
	}
	// number string is reversed, so re-reverse it.
	return reverse(dst, n, pad(p, buf, width), buf);
}

#include "va-printk.h"

int va_printk(char *buf, int n, const char *fmt, va_list args) {
	char *p = buf, *e = buf + n - 1;

	buf[0] = 0;
	for(; *fmt && p < e; ) {
		if(*fmt != '%')
			*p++ = *fmt++;
		
		else if(fmt[1] == '%') {
			*p++ = *fmt;
			fmt+=2;
		} else {	
			fmt++;

			char *s, num[128];
			unsigned width = 0;

			while(isdigit(*fmt)) {
				width = width*10 + *fmt - '0';
				fmt++;
			}
			assert(width < 64);
			
			switch(*fmt) {
			case 'f':{
				double d = va_arg(args, double);
				s = __emit_float(num, d, width);
				break;
			}
			case 'd':
				s = emit(10, num, 128, va_arg(args, int), width, 1);
				break;
			case 'u':
				s = emit(10, num, 128, va_arg(args, uint64_t), width, 0);
				break;
			case 'p':
			case 'x':
				s = emit(16, num, 128, va_arg(args, uint64_t), width, 0);
				break;
			case 'b':
				s = emit(2, num, 128, va_arg(args, uint64_t), width, 0);
				break;
			case 's':
				s = va_arg(args, char *);
				break;
			case 'c':
				s = emit(8, num, 128, va_arg(args, int),width,0);
				break;
			default: 
				panic("printk: not handling specifier '%c'\n", *fmt);
				return 0;   // ugh
			}
			fmt++;

			// safe string copy
			for(; p < e && *s; )
				*p++ = *s++;
		}
	}
	*p++ = 0;
	return p - buf;
}

char *strcat (char *dest, const char *src) {
    char *s1 = dest;
    const char *s2 = src;
    char c;

    /* Find the end of the string.  */
    do
        c = *s1++;
    while (c != '\0');

    /* Make S1 point before the next character, so we can increment
       it while memory is read (wins on pipelined cpus).  */
    s1 -= 2;
    do {
        c = *s2++;
        *++s1 = c;
    } while (c != '\0');
    return dest;
}

// get the integer part of fp number.
static long trunc(double d) {
        return (long)d;
}

// we get 8 digits of precision.
static unsigned fp_get_frac(double d) {
	if(d < 0)
		d = -d;

        return trunc(d * 100000000.) % 100000000;
}

static long fp_get_integral(double d) {
        return trunc(d);
}

// pretty ugly.
char * __emit_float(char *num, double d, unsigned width) {
	char *p = num;

	if(d < 0) {
		*p++ = '-';
		d = -d;
	}
	*p = 0;
		
	unsigned frac = fp_get_frac(d);
	int integral = fp_get_integral(d);

	char tmp[128];
	emit(10, tmp, 128,  integral, width, 1);
	strcat(p, tmp);

	strcat(p, ".");

	const unsigned frac_len = 8;
	emit(10, tmp, 128,  frac, frac_len, 0);

	// have to add leading zeros if small.
	for(int i = 0; i < frac_len; i++)
		if(tmp[i] == ' ')
			tmp[i] = '0';
	strcat(p, tmp);
	return num;
}
