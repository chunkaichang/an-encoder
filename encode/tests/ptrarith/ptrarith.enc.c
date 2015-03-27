
void ___enc_copy(unsigned long a,
                 unsigned long b,
                 long size) {
  for (long i = 0; i < size; i++) {
      long *a_ptr = (long*)(a + i * sizeof(long));
      long *b_ptr = (long*)(b + i * sizeof(long));
      *a_ptr = *b_ptr;
  }
}
