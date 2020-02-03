#include "printf.h"
#include "printf_internal.h"
#include "string.h"
#include <stdarg.h>

#define MAX_OUTPUT_LEN 1024

int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, int min_width)
{
	return signed_to_base(buf, bufsize, val, base, min_width);
}

int signed_to_base(char *buf, size_t bufsize, int val, int base, int min_width)
{
	char *num = "";
	int num_len = 0; // max number of characters written into buffer
	
	// multiplier to keep value positive in translation
	int parity = 1;
	if(val < 0){
		parity = -1;
		val *= parity;
		min_width--;
	}

	// turn value into a number
	while(val != 0){
		num[num_len] = val % base;
		num[num_len] += (num[num_len] < 10) ? 48 : 87;
		val /= base;
		num_len++;
	}
	
	// add the necessary padding
	while(num_len < min_width){
		num[num_len] = '0';
		num_len++;
	}
	
	// add negative sign
	if(parity == -1) {
		num[num_len] = '-';
		num_len++;
	}

	// only add what fits in the buffer
	for(int j = 0; j < bufsize - 1; j++){
		buf[j] = num[num_len - j - 1];
	}
	buf[bufsize - 1] = '\0';

	return num_len;
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args)
{
    /* TODO: Your code here */
    return 0;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...)
{
	int str_len = 0;
	va_list ap;
	va_start(ap, *format);
	for(int i = 0; i < strlen(format); i++){
		if(format[i] == '%'){
			char code = format[i + 1];
			if(code == '%'){
				buf[str_len] = '%';
				i++;
			} else if(code == 'c'){
				buf[str_len] = (char) va_arg(ap, int);
				i++;
			} else if(code == 's'){
				i += strlcat(buf, va_arg(ap, char *));
			}
		} else{
			buf[str_len] = format[i];
			str_len++;
		}	
	}
	
	buf[str_len] = '\0';
	str_len++;
	va_end(ap);
    return str_len;
}

int printf(const char *format, ...)
{
    /* TODO: Your code here */
    return 0;
}
