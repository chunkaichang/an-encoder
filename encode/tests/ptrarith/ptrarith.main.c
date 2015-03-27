#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

extern void ___enc_copy(long, long, long);


int main() {
  unsigned long t1, t2, total = 0;

  const unsigned size = 1024;
  long *a = (long*)malloc(size * sizeof(long));
  long *b = (long*)malloc(size * sizeof(long));

  srand(0);

  for (unsigned i = 0; i < size; i++) {
    b[i] = AN_ENCODE_VALUE(rand());
    a[i] = AN_ENCODE_VALUE(0);
  }

  __cs_reset();

  unsigned long ai = (unsigned long)a;
  unsigned long bi = (unsigned long)b;

  t1 = __cyc_rdtsc();
  ___enc_copy(ai, bi, size);
  t2 = __cyc_rdtsc();
  total += t2 - t1;

  for (unsigned i = 0; i < size; i++)
    __cs_acc(AN_DECODE_VALUE(a[i]));

  __cyc_msg(total);
  __cs_msg();

  return 0;
}
