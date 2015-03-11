#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define assert(x)

#define REPETITIONS 1
#define MAXRANDINT  100

#define LEN_L 10000     // 10 thousand

void mysrand(unsigned int);
int myrand(void);

// test arrays of small, medium and large sizes
int a_l[LEN_L];
int c = 0;
int less = 0;
// perf function: sort array using quicksort
void quicksort(int *array, int start, int end) {
    if (start < end) {
        int l = start+1, r = end, p = array[start];

        while(l < r) {
            if(array[l] <= p)
                l++;
            else if(array[r] >= p) {
                r--;
                assert(r >= 0);
            }
            else {
                int t = array[l];
                array[l] = array[r];
                array[r] = t;
            }
        }

        if(array[l] < p) {
            int t = array[l];
            array[l] = p;
            array[start] = t;
            l--;
            assert(l >= 0);
        } else {
            l--;
            assert(l >= 0);
            int t = array[l];
            array[l] = p;
            array[start] = t;
        }
        quicksort(array, start, l);
        quicksort(array, r, end);
    }
}

void precomputation() {}
void postcomputation() {}

int computation(int which, int size) {
    int *arr = 0;
    if (which == 1)
        arr = &a_l[0];
    if (which == 2)
        arr = &a_l[0];
    if (which == 3)
        arr = &a_l[0];

    quicksort(arr, 0, size-1);
    return 0;
}

int main() {
    long long unsigned int t1, t2, total;
    int i, j, sum;
    int repetitions = REPETITIONS;

    mysrand(0);

    // ----- TEST LARGE
    for (i = 0; i < repetitions; i++) {
        int size = sizeof(a_l)/sizeof(int);
        for (j = 0; j < size; j++)
            a_l[j] = myrand();

        precomputation();
        int sum = computation(3, size);
        postcomputation();
    }
    for (i = 0; i < 10; i++)
      printf("a[%d]=%d\n", i, a_l[i]);

    return 0;
}
