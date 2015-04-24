
#include <stdio.h>
#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

long *a;
long *b;
long *mat;

extern void ___enc_kernel(long *, long *, long *, long);


int main(int argc, char **argv) {
  unsigned long t1, t2, total = 0;

  const unsigned size = 1024;
  long *a = (long*)malloc(size * sizeof(long));
  long *b = (long*)malloc(size * sizeof(long));
  long *mat = (long*)malloc(size * size * sizeof(long));

  for (ulong i = 0; i < size; i++) {
    for (ulong j = 0; j < size; j++)
      mat[i*size + j] = AN_ENCODE_VALUE(i*j);

    b[i] = AN_ENCODE_VALUE(i);
    a[i] = 0;
  }
  __cs_fopen(argc, argv);
  __cs_reset();

  t1 = __cyc_rdtsc();
  ___enc_kernel(a, mat, b, size);
  t2 = __cyc_rdtsc();
  total += t2 - t1;

  for (unsigned i = 0; i < size; i++) {
    __cs_facc(AN_DECODE_VALUE(a[i]));
    __cs_acc(AN_DECODE_VALUE(a[i]));
  }

  __cyc_msg(total);
  __cs_fclose();
  __cs_msg();

  return 0;
}
