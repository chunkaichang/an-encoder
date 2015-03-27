
long ___enc_init_and_sum(long *a, long n, long v) {
    long i;
    long sum = 0;

    for (i = 0; i < n; i++) {
        a[i] = v;
    }

    for (i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum;
}

