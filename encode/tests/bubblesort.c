#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define REPETITIONS 100

#define LEN_L 10000     // 10 thousand

extern void mysrand(unsigned int);
extern int myrand(void);

// test arrays of small, medium and large sizes
int a_l[LEN_L];

// perf function: sort array using bubblesort
int bubblesort(int *array, int size) {
    int swapped;
    int i;
    for (i = 1; i < size; i++)
    {
        swapped = 0;    //this flag is to check if the array is already sorted
        int j;
        for(j = 0; j < size - i; j++)
        {
            if(array[j] > array[j+1])
            {
                int temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
                swapped = 1;
            }
        }
        if(!swapped){
            break; //if it is sorted then stop
        }
    }
    return 0;
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

    return bubblesort(arr, size);
}

int main() {
    long long unsigned int t1, t2;
    int i, j, sum;
    int repetitions = REPETITIONS;

    mysrand(0);

    // ----- TEST LARGE
    for (i = 0; i < repetitions; i++) {
        int size = sizeof(a_l)/sizeof(int);
        for (j = 0; j < size; j++)
            a_l[j] = myrand();
            //a_l[j] = size -j;

        precomputation();
        // LLVM-based encoder does its own cycle counting.
        // BUT: enclosing for-loop and the previous for-loop
        // are also taken into account!!
        //t1 = rdtsc();
        int sum = computation(3, size);

        // LLVM-based encoder does its own cycle counting.
        // BUT: enclosing for-loop and the previous for-loop
        // are also taken into account!!
        // total += t2 - t1;
        postcomputation();
    }
    //for (i = 0; i < 10; i++)
    //  printf("a[%d]=%d\n", i, a_l[i]);

    return 0;
}
