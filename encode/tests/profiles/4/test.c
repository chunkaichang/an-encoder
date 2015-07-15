
#include <stdint.h>

int64_t arithmetic(int64_t a, int64_t b) {
  return a + b;
}

int64_t bitwise(int64_t a, int64_t b) {
  return a | b;
}

int64_t comparison(int64_t a, int64_t b) {
  return a < b;
}

char gep(char *a, int64_t b) {
  return a[b];
}

int64_t load(int64_t *a) {
  return *a;
}

void store(int64_t *a, int64_t b) {
  *a = b;
}




