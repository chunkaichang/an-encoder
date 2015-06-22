
void ___enc_copy(unsigned long a,
                 unsigned long b,
                 long size) {
  long *a_ptr = (long*)(a);
  long *b_ptr = (long*)(b);
  for (long i = 0; i < size; i++) {
      a_ptr[i] = b_ptr[i];
  }
}
