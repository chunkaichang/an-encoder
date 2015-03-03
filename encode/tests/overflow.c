
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void kernel(int32_t *a,
            int32_t *mat, int32_t *b,
            int32_t size) {
  for (int32_t i = 0; i < size; i++) {
    for (int32_t j = 0; j < size; j++) {
      a[i] += mat[i*size + j]*b[j];
    }
  }
}

int32_t main() {
  const uint32_t size = 1024;
  int32_t *a = (int32_t*)malloc(size * sizeof(int32_t));
  int32_t *b = (int32_t*)malloc(size * sizeof(int32_t));
  int32_t *mat = (int32_t*)malloc(size * size * sizeof(int32_t));

  for (uint32_t i = 0; i < size; i++) {
    for (uint32_t j = 0; j < size; j++)
      mat[i*size + j] = i*j;

    b[i] = i;
    a[i] = 0;
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
