#include <sys/utils.h>

int strlen(char *str) {
  int i;
  for (i = 0; *str != '\0'; str++)
    i++;
  return i;
}

char *strncpy(char *dest, char *src, int len) {
  int i;
  for (i = 0; i < len && src[i] != '\0'; i++)  {
    dest[i] = src[i];
  }
  return dest;
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

void convert(char *a, unsigned long n, int base, int i) {

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

  while (num) {
    r = num % 10;
    num = num / 10;
    input[i++] = r + 48;
  }

  reverse(input);
}
