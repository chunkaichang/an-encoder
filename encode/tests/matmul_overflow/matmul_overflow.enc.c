
void ___enc_kernel(long *a,
                   long *mat, long *b,
                   long size) {
  for (long i = 0; i < size; i++) {
    for (long j = 0; j < size; j++) {
      a[i] += mat[i*size + j]*b[j];
    }
  }
}

