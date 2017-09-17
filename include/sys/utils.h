#ifndef _UTILS_H
#define _UTILS_H

int strlen(char *str);
char *strncpy(char *dest, char *src, int len);
void *memset(void *dest, int ch, int num_bytes);
void memcpy(void *dest, const void *src, int n);
void convert(char *a, unsigned long n, int base, int i);
void reverse(char *a);
void int2char(int num, char *input);

#endif
