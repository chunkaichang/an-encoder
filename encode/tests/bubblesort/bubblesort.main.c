/* Adapted from Dmitry Kuvayskiy's test for his
   "AN-Transformer".
 */

#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

#define REPETITIONS 100
#define LEN         1000

#define MAXRANDINT  LEN

long a[LEN];

extern long ___enc_bubblesort(long *array, long size);

int main() {
    uint64_t t1, t2, total = 0;
    unsigned i, j;

    __cs_reset();

    srand(0);
    for (i = 0; i < REPETITIONS; i++) {
        for (j = 0; j < LEN; j++)
          a[j] = AN_ENCODE_VALUE(rand() % MAXRANDINT);

        t1 = __cyc_rdtsc();
        ___enc_bubblesort(a, LEN);
        t2 = __cyc_rdtsc();
        total += t2 - t1;

        for (j = 0; j < LEN; j++)
          __cs_acc(AN_DECODE_VALUE(a[j]));
    }

    __cyc_msg(total);
    __cs_msg();

    return 0;
}
