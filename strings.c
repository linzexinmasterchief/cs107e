#include "strings.h"

const int TRUE = 1;
const int FALSE = 0;
const int NUM_OFFSET = 48;
const int LET_OFFSET = 87;
 
void *memset(void *s, int c, size_t n)
{
	char *s_temp = (char *)s;
	c = (unsigned char) c;
	for(int i = 0; i < n; i++){
		s_temp[i] = c;
	}
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	char *dst_temp = (char *)dst;
	char *src_temp = (char *)src;
	for(int i = 0; i < n; i++){
		dst_temp[i] = src_temp[i];
	}
	return dst;
}

size_t strlen(const char *s)
{
    /* Implementation a gift to you from lab3 */
    int n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

int strcmp(const char *s1, const char *s2)
{
	int n = 0;
	while(s1[n] != '\0'){
		if((s1[n] - s2[n]) != 0) 
			return s1[n] - s2[n];
		n++;
	}
    return s1[n] - s2[n];
}

size_t strlcat(char *dst, const char *src, size_t maxsize)
{
	int dst_len = strlen(dst);
	int cp_max = maxsize - dst_len - 1;
	memcpy(dst + dst_len, src, cp_max);
	dst[maxsize - 1] = '\0';
    return 0;
}

int is_hex(const char *str){
	return str[0] == '0' && str[1] == 'x' ? TRUE : FALSE;
}

int is_dig(char c){
	return '0' <= c && c <= '9' ? TRUE : FALSE;
}

int is_let(char c){
	return 'a' <= c && c <= 'f' ? TRUE : FALSE;
}

unsigned int strtonum(const char *str, const char **endptr)
{
	*endptr = str;
	int base;
	const char *curr_ptr = str;	
	if(is_hex(str)){
		curr_ptr += 2;
		base = 16;
	} else{
		base = 10;
	}

	int num = 0;
	int curr_dig;
	while(*curr_ptr != '\0' && (is_dig(*curr_ptr) || is_let(*curr_ptr))){
		if(is_let(*curr_ptr) && base == 10) return num;

		curr_dig = *curr_ptr;
		curr_dig -= is_let(curr_dig) ? LET_OFFSET : NUM_OFFSET;
		num = num * base + curr_dig;

		curr_ptr += 1;
	}	
    return num;
}
