#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

extern void ___enc_copy(long, long, long);


int main(int argc, char **argv) {
  unsigned long t1, t2, total = 0;

  fprintf(stderr, "LENGTH=%d\n", LENGTH);
  const unsigned size = LENGTH;
  long *a = (long*)malloc(size * sizeof(long));
  long *b = (long*)malloc(size * sizeof(long));

  __cs_log(argc, argv);
  __cs_fopen(argc, argv);
  __cs_reset();

  srand(0);
  for (unsigned r = 0; r < REPETITIONS; r++) {
    for (unsigned i = 0; i < size; i++) {
      b[i] = AN_ENCODE_VALUE(i);
      a[i] = AN_ENCODE_VALUE(0);
    }

    unsigned long ai = (unsigned long)a;
    unsigned long bi = (unsigned long)b;

    __cyc_warmup();
    t1 = __cyc_rdtsc();
    ___enc_copy(ai, bi, size);
    t2 = __cyc_rdtscp();
    total += t2 - t1;

    for (unsigned i = 0; i < size; i++) {
      __cs_facc(AN_DECODE_VALUE(a[i]));
      __cs_acc(AN_DECODE_VALUE(a[i]));
    }
  }

  __cyc_msg(total);
  __cs_fclose();
  __cs_msg();

  return 0;
}
