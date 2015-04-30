
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef MAJORITY_VOTING
  #define VOTES 1
#else
  // We use five votes since a single erroneous write (of 32 bits, i.e. 4 bytes) can affect
  // up to two votes. Hence if we used 3 votes only, we would not be guaranteed a majority
  // after an erroneous write.
  #define VOTES 5
#endif /* MAJORITY_VOTING */

static __uint128_t checksum[VOTES];
static FILE *fp_checksum[VOTES];

static FILE *csl = NULL; // File for logging output to do with votes.

static void set_votes(void *votes, void *value, size_t sz_val) {
  char *addr = (char*)votes;
  for (unsigned i = 0; i < VOTES; i++)
    memcpy(addr + i*sz_val, value, sz_val);
}

void print_votes(FILE *f, void *votes, size_t sz_val) {
  char *addr = (char*)votes;
  fprintf(f, "&votes=0x%lX, votes: ", (uint64_t)votes);
  for (unsigned i = 0; i < VOTES; i++) {
    __uint128_t v = 0;
    memcpy(&v, addr + i*sz_val, sz_val);
    uint64_t lo = (uint64_t)v;
    uint64_t hi = (uint64_t)(v >> 64);
    if (hi) fprintf(f, "0x%lX%lX, ", hi, lo);
    else fprintf(f, "0x%lX, ", lo);
  }
  fprintf(f, "\n");
}

__uint128_t vote(void *votes, size_t sz_val) {
#ifndef MAJORITY_VOTING
  __uint128_t result = 0;
  memcpy(&result, votes, sz_val);
  return result;
#else
  // turn the votes into integers:
  __uint128_t ui_votes[VOTES] = {0};
  char *addr = (char*)votes;
  for (unsigned i = 0; i < VOTES; i++)
    memcpy(ui_votes + i, addr + i*sz_val, sz_val);

  // bubblesort the votes:
  for (unsigned i = 1; i < VOTES; i++) {
      unsigned swapped = 0;
      for(unsigned j = 0; j < VOTES - i; j++) {
          if(ui_votes[j] > ui_votes[j+1]) {
              long temp = ui_votes[j];
              ui_votes[j] = ui_votes[j+1];
              ui_votes[j+1] = temp;
              swapped = 1;
          }
      }
      if(!swapped)
          break;
  }

  // locate blocks of different votes:
  unsigned blocks[VOTES] = {0};
  unsigned blk_index = 0;
  __uint128_t prev = ui_votes[0];
  for (unsigned i = 1; i < VOTES; i++) {
    if (ui_votes[i] != prev) {
      blocks[++blk_index] = i;
      prev = ui_votes[i];
    }
  }
  // return early if there is only one block:
  if (blk_index == 0)
    return ui_votes[0];
  // otherwise find the biggest block:
#ifdef VERBOSE_VOTING
  fprintf(csl, "Multiple votes!!\n");
#endif /* VERBOSE_VOTING */
  unsigned blk_count = blk_index+1 ;
  unsigned max_index = 0;
  unsigned max = blocks[1];
  for (unsigned i = 1; i < blk_count; i++) {
    unsigned size;
    if (i+1 == blk_count)
      size = VOTES - blocks[i];
    else
      size = blocks[i+1] - blocks[i];
    if (size > max)
      max_index = i;
  }
#ifdef VERBOSE_VOTING
  fprintf(csl, "vote (before broadcast) -- ");
  print_votes(csl, votes, sz_val);
#endif /* VERBOSE_VOTING */
  // broadcast majority value:
  __uint128_t broadcast = ui_votes[blocks[max_index]];
  set_votes(votes, &broadcast, sz_val);
#ifdef VERBOSE_VOTING
  fprintf(csl, "vote (after broadcast) -- ");
  print_votes(csl, votes, sz_val);
#endif /* VERBOSE_VOTING */
  return broadcast;
#endif /* MAJORITY_VOTING */
}

__uint128_t __cs_reset() {
  __uint128_t zero = 0;
  set_votes(&checksum, &zero, sizeof(__uint128_t));
  return checksum[0];
}

__uint128_t __cs_get() {
  return vote(&checksum, sizeof(__uint128_t));
}

__uint128_t __cs_acc(__uint128_t x) {
#ifdef VERBOSE_VOTING
  fprintf(csl, "__cs_acc() -- ");
  print_votes(csl, &checksum, sizeof(__uint128_t));
#endif /* VERBOSE_VOTING */
  __uint128_t cs = __cs_get();
  cs = cs * 37 + x;
  set_votes(&checksum, &cs, sizeof(__uint128_t));
  return checksum[0];
}

void __cs_msg(void) {
  __uint128_t cs = __cs_get();
  uint64_t hi, lo;
  hi = cs >> 64;
  lo = (cs << 64) >> 64;
  fprintf(stderr, "hi(checksum)=0x%016lX\n", hi);
  fprintf(stderr, "lo(checksum)=0x%016lX\n", lo);
}

void __cs_fopen(int argc, char** argv) {
  const char *file = "cs.out";
  unsigned i = 0;

  while (i < argc) {
    if (!strcmp("--cso", argv[i])) {
      if (argc <= i+1) exit(1);
      file = argv[i+1];
      break;
    }
    ++i;
  }
  FILE *fp = fopen(file,"w");
  if (!fp) exit(1);
  set_votes(fp_checksum, &fp, sizeof(FILE*));
}

size_t __cs_facc(__uint128_t x) {
#ifdef VERBOSE_VOTING
  fprintf(csl, "__cs_facc() -- ");
  print_votes(csl, fp_checksum, sizeof(FILE*));
#endif /* VERBOSE_VOTING */
  FILE *fp = (FILE*)vote(fp_checksum, sizeof(FILE*));

  uint64_t x64[2];
  x64[0] = (uint64_t)x;
  x64[1] = (uint64_t)(x >> 64);
  size_t r = fwrite(x64, sizeof(uint64_t), 2, fp);
  return r;
}

void __cs_fclose() {
  FILE *fp = (FILE*)vote(fp_checksum, sizeof(FILE*));
  fclose(fp);
}

FILE* __cs_log(int argc, char** argv) {
#ifdef VERBOSE_VOTING
  csl = stderr;
  unsigned i = 0;

  while (i < argc) {
    if (!strcmp("--csl", argv[i])) {
      if (argc <= i+1) exit(1);
      csl = fopen(argv[i+1], "w");
      break;
    }
    ++i;
  }
#endif /* VERBOSE_VOTING */
  return csl;
}
