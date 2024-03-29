#include <sys/utils.h>
#include <sys/defs.h>

int strlen(char *str) {
  int i;
  for (i = 0; *str != '\0'; str++)
    i++;
  return i;
}

char *strcpy(char *dest, char *src) {
  int i;
  for (i=0; src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  dest[i] = '\0';
  return dest;
}

char *strncpy(char *dest, char *src, int len) {
  int i;
  for (i = 0; i < len && src[i] != '\0'; i++)  {
    dest[i] = src[i];
  }
  return dest;
}

void clear_chars(char *temp, int size) {
  int i = 0;
  for (i = 0; i < size; i += 2) {
     temp[i] = 0;
  }
}

void *memset(void *dest, int ch, int num_bytes) {
  char *tmp = dest;
  while (num_bytes) {
    *tmp++ = ch;
    num_bytes--;
  }
  return dest;
}

void memcpy(void *dest, const void *src, int n) {
  char *d = (char *)dest;
  char *s = (char *)src;
  while (n) {
    *d = *s;
    d++;
    s++;
    n--;
  }
}

/* number base converter, int to oct, hex etc */
void convert(char *a, unsigned long n, int base, int i) {
  if (n == 0 && i == 0) {
    a[i] = 48;
    return;
  }

  int rem = n % base;
  if (n == 0)
    return;

  convert(a, n / base, base, i + 1);
  if (rem < 10) {
    a[i] = rem + 48;

  } else {
    a[i] = rem - 10 + 'A';
  }
}

void reverse(char *a) {
  int i;
  int j = strlen(a);
  char c;

  for (i = 0,j = strlen(a) - 1; i < j; i++, j--) {
    c = a[i];
    a[i] = a[j];
    a[j] = c;
  }
}

void int2char(int num, char *input) {
  int i = 0, r;
  if (num == 0) {
    input[i++] = 48;

  } else {

    while (num) {
      r = num % 10;
      num = num / 10;
      input[i++] = r + 48;
    }
  }

  reverse(input);
}

int strcmp (const char *f_str, const char *s_str) {

  while (*f_str && (*f_str == *s_str)) {
    f_str++; s_str++;
  }
  return (*(unsigned char *)f_str - *(unsigned char *)s_str);
}

int strncmp (const char *f_str, const char *s_str, int n) {
  for ( ; n > 0; f_str++, s_str++, n--) {
    if (*f_str != *s_str) {
      return (*(unsigned char *)f_str < *(unsigned char *)s_str) ? -1 : 1;
    }
    else if(*f_str == '\0') {
      return 0;
    }
  }
  return 0;
}

char *strchr(char *s, int c)
{
    char* ret=0;
    do {
        if( *s == (char)c )
            ret=s;
    } while(*s++);
    return ret;
}

size_t strspn(char *s1, char *s2)
{
    size_t ret=0;
    while(*s1 && strchr(s2,*s1++))
        ret++;
    return ret;
}

size_t strcspn(char *s1, char *s2)
{
    size_t ret=0;
    while(*s1)
        if(strchr(s2,*s1))
            return ret;
        else
            s1++,ret++;
    return ret;
}

char *strtok_r(char *str, char *delim, char **nextp) {
  char *ret;
  if (str == NULL)
    str = *nextp;

  str += strspn(str, delim);
  if (*str == '\0')
    return NULL;

  ret = str;
  str += strcspn(str, delim);
  if (*str)
    *str++ = '\0';

  *nextp = str;
  return ret;
}
