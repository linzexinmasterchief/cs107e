#include "printf_internal.h"
#include <stdarg.h>

#define MAX_OUTPUT_LEN 1024

const int NUM_OFFSET = 48; // TODO: check
const int LET_OFFSET = 87; // TODO: check

int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, int min_width)
{
	char *num = "";
	int i = 0;
	while(val != 0){
		num[i] = val % base + NUM_OFFSET;
		val /= base;
		i++;
	}
	
	int num_len = strlen(num);
	for(int j = 0; j < bufsize - 1; j++){
		buf[j] = num[num_len - j - 1];
	}
	num[bufsize - 1] = '\0';

	return i;
}

int signed_to_base(char *buf, size_t bufsize, int val, int base, int min_width)
{
    /* TODO: Your code here */
    return 0;
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args)
{
    /* TODO: Your code here */
    return 0;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...)
{
    /* TODO: Your code here */
    return 0;
}

int printf(const char *format, ...)
{
    /* TODO: Your code here */
    return 0;
}
