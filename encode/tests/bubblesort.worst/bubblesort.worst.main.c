/* Adapted from Dmitry Kuvayskiy's test for his
   "AN-Transformer".
 */

#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

#define MAXRANDINT  LENGTH

long a[LENGTH];

extern long ___enc_bubblesort(long *array, long size);

int main(int argc, char** argv) {
    uint64_t t1, t2, total = 0;
    unsigned i, j;

    fprintf(stderr, "LENGTH=%d\n", LENGTH);
    
    __cs_log(argc, argv);
    __cs_fopen(argc, argv);
    __cs_reset();

    srand(0);
    for (i = 0; i < REPETITIONS; i++) {
        for (j = 0; j < LENGTH; j++)
          a[j] = AN_ENCODE_VALUE(LENGTH - j);

	__cyc_warmup();
        t1 = __cyc_rdtsc();
        ___enc_bubblesort(a, LENGTH);
        t2 = __cyc_rdtscp();
        total += t2 - t1;

        for (j = 0; j < LENGTH; j++) {
          __cs_facc(AN_DECODE_VALUE(a[j]));
          __cs_acc(AN_DECODE_VALUE(a[j]));
        }
    }

    __cyc_msg(total);
    __cs_fclose();
    __cs_msg();

    return 0;
}
