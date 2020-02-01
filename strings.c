#include "strings.h"

void *memset(void *s, int c, size_t n)
{
	s = (char *)s;
	c = (unsigned char) c;
	for(int i = 0; i < n; i++){
		s[i] = c;
	}
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	dst = (char *)dst;
	src = (char *)src;
	for(int i = 0; i < n; i++){
		dst[i] = src[i];
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
    return s2[n] - s1[n];
}

size_t strlcat(char *dst, const char *src, size_t maxsize)
{
    /* TODO: Your code here */
    return 0;
}

unsigned int strtonum(const char *str, const char **endptr)
{
    /* TODO: Your code here */
    return 0;
}
