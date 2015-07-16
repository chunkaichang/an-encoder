

long ___enc_bubblesort(long *array, long size) {
    long swapped;
    long i;
    for (i = 1; i < size; i++)
    {
        swapped = 0;    //this flag is to check if the array is already sorted
        long j;
        for(j = 0; j < size - i; j++)
        {
            if(array[j] > array[j+1])
            {
                long temp = array[j];
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

