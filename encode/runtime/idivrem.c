
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void idivrem(int64_t *div, int64_t *rem, int64_t x, int64_t y) {
  int64_t _div, _rem;
  __asm__ __volatile__ ("cqto\n"
                        "idivq %3\n"
			"movq %%rax, %0\n"
			"movq %%rdx, %1\n"
                        : "=m" (_div), "=m" (_rem)
                        : "a" (x), "m" (y)
                        : "%rax", "%rdx");
  *div = _div; *rem = _rem;
}

void condexit(int64_t x) {
  if (x) exit(2);
}
