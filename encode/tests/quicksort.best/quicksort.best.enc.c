
void quicksort(long *array, long start, long end) {
    if (start < end) {
        long l = start+1, r = end, p = array[start];

        while(l < r) {
            if(array[l] <= p) {
                l++;
            } else if(array[r] >= p) {
                r--;
            } else {
                long t = array[l];
                array[l] = array[r];
                array[r] = t;
            }
        }

        if(array[l] < p) {
            long t = array[l];
            array[l] = p;
            array[start] = t;
            l--;
        } else {
            l--;
            long t = array[l];
            array[l] = p;
            array[start] = t;
        }
        quicksort(array, start, l);
        quicksort(array, r, end);
    }
}

void ___enc_quicksort(long *array, long start, long end) {
  quicksort(array, start, end);
}
