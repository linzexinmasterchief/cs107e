#include "printf.h"
#include "printf_internal.h"
#include "strings.h"
#include <stdarg.h>
#include "uart.h"

#define MAX_OUTPUT_LEN 1024

int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, int min_width)
{
	return signed_to_base(buf, bufsize, (unsigned int)val, base, min_width);
}

int signed_to_base(char *buf, size_t bufsize, int val, int base, int min_width)
{
	char num[33 + min_width]; // interpret as MAX_INT_SIZE + str_len('\0') + min_width
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
		num[num_len] += (num[num_len] < 10) ? 48 : 87; // adds according to whether we have a letter or number
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

	// only keep what fits in the buffer
	int j = 0;
	for(;j < bufsize - 1; j++){
		if(j == num_len) break;
		buf[j] = num[num_len - j - 1];
	}
	buf[j] = '\0';

	return num_len;
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args)
{
	int str_len = 0;
	unsigned int width = 0;
	for(int i = 0; i < strlen(format); i++){
		if(format[i] == '%'){
			switch (format[i + 1]) {
				case '%':
					buf[str_len] = '%';
					str_len++;
					break;
				case 'c':
					buf[str_len] = (char) va_arg(args, int);
					str_len++;
					break;
				case 's':
					str_len += strlcat(buf, va_arg(args, char *), bufsize - str_len);
					break;

				/*
				* For the digit cases note that we are writing directly to the end of the buffer,
				* starting at the '/0'. str_len informs where we begin and how much space we have
				* available to write to. 
				*/
				case '0':;
					const char *temp;
					width = strtonum(format + i + 1, &temp);
					i += temp - format - 1; // temp is set to start at the digit type, the - 1 is for consistency
					if(format[i + 1] == 'd')
						str_len += signed_to_base(buf + str_len, bufsize - str_len, va_arg(args, int), 10, width); 
					else
						str_len += unsigned_to_base(buf + str_len, bufsize - str_len, va_arg(args, int), 16, width); 
					break;
				case 'd':
					str_len += signed_to_base(buf + str_len, bufsize - str_len, va_arg(args, int), 10, 0); 
					width = 0;
					break;
				case 'x':
					str_len += unsigned_to_base(buf + str_len, bufsize - str_len, va_arg(args, unsigned int), 16, 0); 
					width = 0;
					break;

				case 'p':
					// add the formatting character and increment str_len all in one go
					buf[str_len++] = '0'; 
					buf[str_len++] = 'x';
					str_len += unsigned_to_base(buf + str_len, bufsize - str_len, (va_arg(args, unsigned int)), 16, 0);
					break;
			}
			i++;
		}
		else{
			buf[str_len] = format[i];
			str_len++;
		}	
		buf[str_len] = '\0'; // IMPORTANT: Don't forget to null terminate after every addition
	}
    return ++str_len; // return incremented value because of last null terminator
}

int snprintf(char *buf, size_t bufsize, const char *format, ...)
{
	buf[0] = '\0';

	va_list args;
	va_start(args, format);
	int str_len = vsnprintf(buf, bufsize, format, args); 
	va_end(args);

    return str_len;
}

int printf(const char *format, ...)
{
	char print[MAX_OUTPUT_LEN];
	
	va_list args;
	va_start(args, format);
	int str_len = vsnprintf(print, MAX_OUTPUT_LEN, format, args); 
	va_end(args);

	uart_putstring(print);
    return str_len;
}
