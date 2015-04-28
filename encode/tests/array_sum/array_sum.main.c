/* Adapted from Dmitry Kuvayskiy's test for his
   "AN-Transformer".
 */

#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

extern long ___enc_init_and_sum(long*, long, long);

long a[LENGTH];

int main(int argc, char** argv) {
    uint64_t t1, t2, total = 0;
    unsigned i;

    __cs_log(argc, argv);
    __cs_fopen(argc, argv);
    __cs_reset();

    for (i = 0; i < REPETITIONS; i++) {
      long sum;

      t1 = __cyc_rdtsc();
      sum = ___enc_init_and_sum(a, LENGTH, 42);
      t2 = __cyc_rdtsc();
      total += t2 - t1;

      __cs_facc(sum);
      __cs_acc(sum);
    }

    __cyc_msg(total);
    __cs_fclose();
    __cs_msg();

    return 0;
}
