
#include <stdint.h>
#include <stdio.h>

static __uint128_t checksum;

__uint128_t __cs_reset() {
  checksum = 0;
  return checksum;
}

__uint128_t __cs_get() {
  return checksum;
}

__uint128_t __cs_acc(__uint128_t x) {
  checksum = checksum * 37 + x;
  return checksum;
}

void __cs_msg(void) {
  __uint128_t cs = __cs_get();
  uint64_t hi, lo;
  hi = cs >> 64;
  lo = (cs << 64) >> 64;
  fprintf(stderr, "hi(checksum)=0x%016lX\n", hi);
  fprintf(stderr, "lo(checksum)=0x%016lX\n", lo);
}
