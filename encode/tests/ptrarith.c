
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void copy(int64_t a,
          int64_t b,
          int32_t size) {
  for (int32_t i = 0; i < size; i++) {
      uint32_t *a_ptr = (uint32_t*)(a + i * sizeof(uint32_t));
      uint32_t *b_ptr = (uint32_t*)(b + i * sizeof(uint32_t));
      *a_ptr = *b_ptr;
  }
}

int32_t main() {
  const uint32_t size = 1024;
  int32_t *a = (int32_t*)malloc(size * sizeof(int32_t));
  int32_t *b = (int32_t*)malloc(size * sizeof(int32_t));

  for (uint32_t i = 0; i < size; i++) {
    b[i] = i;
    a[i] = 0;
  }

  int64_t ai = (int64_t)a;
  int64_t bi = (int64_t)b;
  copy(ai, bi, size);
  // Should be commented out for performance measurement:
  /*   for (uint32_t i = 0; i < 16; i++) {
   *    printf("a[%02d]=%d\n", i, a[i]);
   *  }
   */
  return 0;
}
