/* Adapted from Dmitry Kuvayskiy's test for his
   "AN-Transformer".
 */

#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

#define REPETITIONS 100
#define LEN         10000000 /* 10 million */


extern long ___enc_init_and_sum(long*, long, long);

long a[LEN];

int main() {
    uint64_t t1, t2, total = 0;
    unsigned i;

    __cs_reset();

    for (i = 0; i < REPETITIONS; i++) {
      long sum;

      t1 = __cyc_rdtsc();
      sum = ___enc_init_and_sum(a, LEN, 42);
      t2 = __cyc_rdtsc();
      total += t2 - t1;

      __cs_acc(sum);
    }

    __cyc_msg(total);
    __cs_msg();

    return 0;
}
