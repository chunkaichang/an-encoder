
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static __uint128_t checksum;
static FILE *fp_checksum;

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

void __cs_fopen(int argc, char** argv) {
  const char *file = "cs.out";
  unsigned i = 0;

  while (i < argc) {
    if (!strcmp("--cso", argv[i])) {
      if (argc <= i+1) exit(1);
      file = argv[i+1];
      break;
    }
    ++i;
  }
  fp_checksum = fopen(file, "w");
}

size_t __cs_facc(uint64_t x) {
  return fwrite(&x, sizeof(__uint64_t), 1, fp_checksum);
}

void __cs_fclose() {
  fclose(fp_checksum);
}
