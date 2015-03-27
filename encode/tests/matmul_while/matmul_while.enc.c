
void ___enc_kernel(long *a,
                  long *mat, long *b,
                  long size) {
  long i = 0;
  while (1) {
    if (!(i < size))
      break;

    long j = 0;
    while (1) {
      if (!(j < size))
        break;
      a[i] += mat[i*size + j]*b[j];
      ++j;
    }
    ++i;
  }
}
