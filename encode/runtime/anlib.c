/* Based on 'an_encoder.py' from the previous, Python-based implementation of an "AN Encoder".
 * Path: <PROJECT ROOT DIRECTORY>/transformer/encoders/an_encoder.py
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../tests/mylibs/an_builtins.h"

typedef uint64_t ptr_enc_t;

static const uint64_t A = CODE_VALUE_A;
// We do 'signed remainder' to check validity of coded values.
// Hence the accumulator should probably be signed too.
static int64_t accu_enc = 0;

#ifdef LOG_ACCU
  #define LOG_ACCU_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
  #define LOG_ACCU_PRINTF(...)
#endif 

void ___accumulate_ignore_oflow_enc(int64_t x_enc, int64_t *accu)
{
  LOG_ACCU_PRINTF("x_enc=0x%016lx, accu=0x%016lx\n", x_enc, *accu);
  *accu += x_enc;
}

void accumulate_ignore_oflow_enc(int64_t x_enc)
{
  ___accumulate_ignore_oflow_enc(x_enc, &accu_enc);
}

void ___accumulate_enc(int64_t x_enc, int64_t *accu)
{
  int64_t old_accu = *accu;
  LOG_ACCU_PRINTF("x_enc=0x%016lx, old_accu=0x%016lx, ", x_enc, old_accu);

  if(__builtin_saddl_overflow(old_accu, x_enc, accu)) {
    LOG_ACCU_PRINTF("OF, new_accu=0x%016lx", *accu);
    an_assert(old_accu, A);
    *accu = x_enc;
  }
  LOG_ACCU_PRINTF("\n");
}

void accumulate_enc(int64_t x_enc)
{
  ___accumulate_enc(x_enc, &accu_enc);
}

int64_t add_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  r_enc = x_enc + y_enc;
  return r_enc;
}
int64_t sub_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  r_enc = x_enc - y_enc;
  return r_enc;
}
int64_t mul_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);

  const int64_t p = 278;
  const int64_t q = 211;

  r_enc = ((x_enc - x)/p) * ((y_enc - y)/q) + x*y;

  return r_enc;
}
int64_t udiv_enc(uint64_t x_enc, uint64_t y_enc)
{
  int64_t r_enc = 0;

  uint64_t x = x_enc/A;
  uint64_t y = y_enc/A;
  uint64_t trunc_correction = x % y;

  x_enc -= an_encode(trunc_correction, A);

  //__uint128_t tmp = 0;
  //tmp = an_encode((__uint128_t)x_enc * (__uint128_t) A, A) / (__uint128_t) y_enc;
  uint64_t tmp = an_encode(x_enc, A) / y_enc;

  r_enc = (int64_t) tmp;
  return r_enc;
}
int64_t sdiv_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;

  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  int64_t tmp = x / y;

  r_enc = an_encode(tmp, A);
  return r_enc;
}
int64_t umod_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  uint64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  r_enc = an_encode(x % y, A);
  return r_enc;
}
int64_t smod_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  r_enc = an_encode(x % y, A);
  return r_enc;
}
int64_t eq_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  if (x_enc == y_enc)
    r_enc = A;
  return r_enc;
}
int64_t neq_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  if (x_enc != y_enc)
    r_enc = A;
  return r_enc;
}
int64_t less_enc(int64_t x_enc, int64_t y_enc) 
{
  int64_t r_enc = 0;
  if (x_enc < y_enc)
    r_enc = A;
  return r_enc;
}
int64_t grt_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  if (x_enc > y_enc)
    r_enc = A;
  return r_enc;
}
int64_t leq_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  if (x_enc <= y_enc)
    r_enc = A;
  return r_enc;
}
int64_t geq_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  if (x_enc >= y_enc)
    r_enc = A;
  return r_enc;
}

ptr_enc_t deref_enc(ptr_enc_t p_enc)
{
  ptr_enc_t r_enc = 0;
  r_enc = an_decode(p_enc, A);
  return r_enc;
}
ptr_enc_t ref_enc(ptr_enc_t addr)
{
  ptr_enc_t r_enc = 0;
  r_enc = an_encode(addr, A);
  return r_enc;
}
ptr_enc_t ptradd_enc(ptr_enc_t p_enc, int64_t x_enc, int64_t size)
{
  ptr_enc_t r_enc = 0;
  r_enc = p_enc + x_enc * size;
  return r_enc;
}
ptr_enc_t ptrsub_enc(ptr_enc_t p_enc, int64_t x_enc, int64_t size)
{
  ptr_enc_t r_enc = 0;
  r_enc = p_enc - x_enc * size;
  return r_enc;
}
int64_t ptrdiff_enc(ptr_enc_t p_enc, ptr_enc_t q_enc, int64_t size)
{
  int64_t r_enc = 0;
  r_enc = (p_enc - q_enc) / size;
  return r_enc;
}
int64_t land_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  r_enc = mul_enc(x_enc, y_enc);
  return r_enc;
}
int64_t lor_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  /* TODO: Find out if performance is better or worse with direct en-/decoding: */
  r_enc = add_enc(x_enc, sub_enc(y_enc, mul_enc(x_enc, y_enc)));
  return r_enc;
}
int64_t and_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  int64_t r_enc = an_encode(x & y, A);
  return r_enc;
}
int64_t or_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  int64_t r_enc = an_encode(x | y, A);
  return r_enc;
}
int64_t xor_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  int64_t r_enc = an_encode(x ^ y, A);
  return r_enc;
}

int64_t onesc_enc(int64_t x_enc)
{
  /* one's complement, i.e. ~x */
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  r_enc = an_encode(~x, A);
  return r_enc;
}
int64_t shl_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  int64_t y = an_decode(y_enc, A);
  r_enc = an_encode(x << y, A);
  return r_enc;
}
int64_t shr_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  uint64_t y = an_decode(y_enc, A);
  r_enc = an_encode(x >> y, A);
  return r_enc;
}
int64_t ashr_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = an_decode(x_enc, A);
  uint64_t y = an_decode(y_enc, A);
  int64_t asr = x >> y;
  r_enc = an_encode(asr, A);
  return r_enc;
}
