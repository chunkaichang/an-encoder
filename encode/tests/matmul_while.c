
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void kernel(int32_t *a,
            int32_t *mat, int32_t *b,
            int32_t size) {
  int32_t i = 0;
  while (1) {
    if (!(i < size))
      break;

    int32_t j = 0;
    while (1) {
      if (!(j < size))
        break;
      a[i] += mat[i*size + j]*b[j];
      ++j;
    }
    ++i;
  }
}

int32_t main() {
  const uint32_t size = 1024;
  int32_t *a = (int32_t*)malloc(size * sizeof(int32_t));
  int32_t *b = (int32_t*)malloc(size * sizeof(int32_t));
  int32_t *mat = (int32_t*)malloc(size * size * sizeof(int32_t));

  int32_t i = 0, j= 0;

  while (1) {
    if (!(i < size))
      break;

    int32_t j = 0;
    while (1) {
      if (!(j < size))
        break;
      mat[i*size + j] = i;
      ++j;
    }
    b[i] = 1;
    a[i] = 0;
    ++i;
  }

  kernel(a, mat, b, size);
  // Should be commented out for performance measurement:
  /*
   *  for (uint32_t i = 0; i < 16; i++) {
   *    printf("a[%02d]=%d\n", i, a[i]);
   *  }
   */
  return 0;
}
