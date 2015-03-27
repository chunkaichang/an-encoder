
#include <stdint.h>
#include <stdio.h>

uint64_t __cyc_rdtsc() {
  uint32_t low, high;
#ifdef __x86_64
  __asm__ __volatile__ ("xorl %%eax,%%eax \n    cpuid"
                        ::: "%rax", "%rbx", "%rcx", "%rdx" );
#else
  __asm__ __volatile__ ("xorl %%eax,%%eax \n    cpuid"
                        ::: "%eax", "%ebx", "%ecx", "%edx" );
#endif
  __asm__ __volatile__ ("rdtsc" : "=a" (low), "=d" (high));
  return (((uint64_t)high) << 32) | (uint64_t)low;
}

void __cyc_msg(uint64_t cs) {
  fprintf(stderr, "elapsed cycles: %lu\n", cs);
}

