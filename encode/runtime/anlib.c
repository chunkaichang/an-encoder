/* Based on 'an_encoder.py' from the previous, Python-based implementation of an "AN Encoder".
 * Path: <PROJECT ROOT DIRECTORY>/transformer/encoders/an_encoder.py
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

typedef uint64_t ptr_enc_t;

static uint64_t A;
static uint64_t accu_enc = 0;

/* NOTE: 'accumulates' have been removed.
 *       => let the compiler figure out when itś wise to insert them.
 */
void signal_enc()
{
  puts("[ERROR]: ANEncoder detected an error!");
}
void invalidate_accu_enc()
{
  accu_enc += 1;
}
void check_enc() {
  if (__builtin_an_check_i32(accu_enc, A)) {
    signal_enc();
    __builtin_an_assert_i32(accu_enc, A);
  }
}
void accumulate_enc(int64_t x_enc)
{
  int64_t  x_mask = x_enc >> 63;
  uint64_t ux_enc = (x_enc + x_mask) ^ x_mask;

  if (ux_enc > UINT64_MAX - accu_enc) {
    accu_enc = 0;
  }
  accu_enc += ux_enc;
  // NOTE: The following assert fails when 'x_enc' overflows. A way to avoid this is to choose a
  // power of 2 as the value of 'A'.
  //__builtin_an_assert_i32(accu_enc, A);
}

int64_t pow2_enc(int64_t x_enc)
{
  /*int64_t powers_enc[] = {
      0x1*(int64_t)A, 0x2*(int64_t)A, 0x4*(int64_t)A, 0x8*(int64_t)A,
      0x10*(int64_t)A, 0x20*(int64_t)A, 0x40*(int64_t)A, 0x80*(int64_t)A,
      0x100*(int64_t)A, 0x200*(int64_t)A, 0x400*(int64_t)A, 0x800*(int64_t)A,
      0x1000*(int64_t)A, 0x2000*(int64_t)A, 0x4000*(int64_t)A, 0x8000*(int64_t)A,
      0x10000*(int64_t)A, 0x20000*(int64_t)A, 0x40000*(int64_t)A, 0x80000*(int64_t)A,
      0x100000*(int64_t)A, 0x200000*(int64_t)A, 0x400000*(int64_t)A, 0x800000*(int64_t)A,
      0x1000000*(int64_t)A, 0x2000000*(int64_t)A, 0x4000000*(int64_t)A, 0x8000000*(int64_t)A,
      0x10000000*(int64_t)A, 0x20000000*(int64_t)A, 0x40000000*(int64_t)A, 0x80000000*(int64_t)A
  };*/

  int64_t r_enc = 0;
  accumulate_enc(r_enc);
  r_enc = 1 << __builtin_an_decode_i64(x_enc, A);
  r_enc = __builtin_an_encode_i64(r_enc, A);
  return r_enc;
}
int64_t add_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  r_enc = x_enc + y_enc;
  accumulate_enc(r_enc);
  // NOTE: As in 'accumulate_enc', the following assert fails if overflow has occured.
  //__builtin_an_assert_i64(r_enc, A);
  return r_enc;
}
int64_t sub_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  r_enc = x_enc - y_enc;
  accumulate_enc(r_enc);
  return r_enc;
}
int64_t mul_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;

  __int128_t tmp = 0;
  tmp = __builtin_an_decode_i128((__int128_t)x_enc * (__int128_t)y_enc, A);

  r_enc = (int64_t) tmp;
  accumulate_enc(r_enc);
  return r_enc;
}
int64_t udiv_enc(uint64_t x_enc, uint64_t y_enc)
{
  int64_t r_enc = 0;

  uint64_t x = x_enc/A;
  uint64_t y = y_enc/A;
  uint64_t trunc_correction = x % y;

  x_enc -= __builtin_an_encode_i64(trunc_correction, A);

  __uint128_t tmp = 0;
  tmp = __builtin_an_encode_i128((__uint128_t)x_enc * (__uint128_t) A, A) / (__uint128_t) y_enc;

  r_enc = (int64_t) tmp;
  accumulate_enc(r_enc);
  return r_enc;
}
int64_t sdiv_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;

  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  int64_t tmp = x / y;

  r_enc = __builtin_an_encode_i64(tmp, A);
  accumulate_enc(r_enc);
  return r_enc;
}
int64_t umod_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  uint64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  r_enc = __builtin_an_encode_i64(x % y, A);
  accumulate_enc(r_enc);
  return r_enc;
}
int64_t smod_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  r_enc = __builtin_an_encode_i64(x % y, A);
  accumulate_enc(r_enc);
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
  r_enc = __builtin_an_decode_i64(p_enc, A);
  return r_enc;
}
ptr_enc_t ref_enc(ptr_enc_t addr)
{
  ptr_enc_t r_enc = 0;
  r_enc = __builtin_an_encode_i64(addr, A);
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
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  int64_t r_enc = __builtin_an_encode_i64(x & y, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  return r_enc;
}
int64_t or_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  int64_t r_enc = __builtin_an_encode_i64(x | y, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  return r_enc;
}
int64_t xor_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  int64_t r_enc = __builtin_an_encode_i64(x ^ y, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  return r_enc;
}

int64_t onesc_enc(int64_t x_enc)
{
  /* one's complement, i.e. ~x */
  int64_t r_enc = 0;
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  r_enc = __builtin_an_encode_i64(~x, A);
  return r_enc;
}
int64_t shl_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  int64_t y = __builtin_an_decode_i64(y_enc, A);
  r_enc = __builtin_an_encode_i64(x << y, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  /* TODO: Find out if performance is better or worse with direct en-/decoding: */
  //r_enc = mul_enc(x_enc, pow2_enc(y_enc));
  return r_enc;
}
int64_t shr_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  uint64_t y = __builtin_an_decode_i64(y_enc, A);
  r_enc = __builtin_an_encode_i64(x >> y, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  /* TODO: Find out if performance is better or worse with direct en-/decoding: */
  //r_enc = udiv_enc(x_enc, pow2_enc(y_enc));
  return r_enc;
}
int64_t ashr_enc(int64_t x_enc, int64_t y_enc)
{
  int64_t r_enc = 0;
  int64_t x = __builtin_an_decode_i64(x_enc, A);
  uint64_t y = __builtin_an_decode_i64(y_enc, A);
  int64_t asr = x >> y;
  r_enc = __builtin_an_encode_i64(asr, A);
  accumulate_enc(r_enc);  // don't really makes sense - r_enc has just been encoded
  /* TODO: Find out if performance is better or worse with direct en-/decoding: */
  /* FIXME: Using 'pow2_enc' here seems to produce wrong result! */
  //r_enc = sdiv_enc(x_enc, pow2_enc(y_enc));
  return r_enc;
}
/* TODO: Find out what this is good for: */
int64_t adjust_downcast_enc(int64_t x_enc, int64_t a_enc)
{
  int64_t r_enc = x_enc;
  if (x_enc >= a_enc/2)
      r_enc = x_enc - a_enc;
  return r_enc;
}
/* TODO: Find out what this is good for: */
int64_t adjust_upcast_enc(int64_t x_enc, int64_t a_enc)
{
  int64_t r_enc = x_enc;
  if (x_enc < 0)
      r_enc = x_enc + a_enc;
  return r_enc;
}
