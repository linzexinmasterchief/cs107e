#include "strings.h"

// Defining boolean constants since we don't have stdbool.h
#define TRUE 1
#define FALSE 0
/* 
* Constants refer to the offset between numbers and characters in the ASCII encoding scheme
* ie: 'a' = 97 so 'a' - LET_OFFSET = 10 
* ie: '0' = 48 so '0' - NUM_OFFSET = 0 
*/
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
	int cp_max = maxsize - dst_len - 1; // max number of appendable characters
	int cp_amt = strlen(src) < cp_max ? strlen(src) : cp_max; // copy the minimum number of characters need
	memcpy(dst + dst_len, src, cp_amt); // starts copying at '\0' of dst
	dst[dst_len + cp_amt] = '\0'; // terminates the new string
    return dst_len + cp_amt + 1; // should be equal to strlen(dst)
}

/*
* Function returns whether a given string should be interpreted as a hexadecimal value
*/
int is_hex(const char *str){
	return str[0] == '0' && str[1] == 'x' ? TRUE : FALSE;
}

/*
* Function returns whether character is a digit
*/
int is_dig(char c){
	return '0' <= c && c <= '9' ? TRUE : FALSE;
}

/*
* Function returns whether character is a valid hexadecimal letter
* NOTE: We only accept lowercase letters 
*/
int is_let(char c){
	return 'a' <= c && c <= 'f' ? TRUE : FALSE;
}

unsigned int strtonum(const char *str, const char **endptr)
{
	*endptr = str;
	int base;

	// set up the function to process either hex or dec string
	if(is_hex(str)){
		*endptr += 2;
		base = 16;
	} else{
		base = 10;
	}

	int num = 0;
	int curr_dig;
	while(**endptr != '\0' && (is_dig(**endptr) || is_let(**endptr))){
		if(is_let(**endptr) && base == 10) return num; // extra care needed for base 10

		curr_dig = **endptr;
		curr_dig -= is_let(curr_dig) ? LET_OFFSET : NUM_OFFSET; // translate char --> int value
		num = num * base + curr_dig;

		*endptr += 1;
	}	
    return num;
}
