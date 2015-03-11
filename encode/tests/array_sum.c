#include <stdio.h>
#include <string.h>

#define REPETITIONS 100

#define LEN_L 10000000     // 10 million

// test arrays of small, medium and large sizes
int a_l[LEN_L];

// perf function: initialize the array and then sum it up
int init_and_sum(int *a, int n, int v) {
    int i;
    int sum = 0;

    for (i = 0; i < n; i++) {
        a[i] = v;
    }

    for (i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum;
}

void precomputation() {}
void postcomputation() {}

int computation(int which, int n, int v) {
    int *arr = 0;
    if (which == 1)
        arr = &a_l[0];
    if (which == 2)
        arr = &a_l[0];
    if (which == 3)
        arr = &a_l[0];

    return init_and_sum(arr, n, v);
}

int main() {
    long long unsigned int t1, t2;
    int i, sum;
    int repetitions = REPETITIONS;

    // ----- INITIALIZE
    precomputation();

    // ----- TEST LARGE
    for (i = 0; i < repetitions; i++) {
        // LLVM-based encoder does its own cycle counting.
        // BUT: for-loop is also taken into account!!
        //t1 = rdtsc();

        int sum = computation(3, sizeof(a_l)/sizeof(int), 10);
        // For verification purposes, comment out for
        // performance measurements:
        //printf("sum: %d\n", sum);

        // LLVM-based encoder does its own cycle counting.
        // BUT: for-loop is also taken into account!!
        //t2 = rdtsc();
        //total += t2 - t1;
    }

    // ----- FINALIZE
    postcomputation();

    return 0;
}
