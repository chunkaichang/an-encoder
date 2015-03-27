#ifndef __ENCODE_H__
#define __ENCODE_H__

#ifdef ENCODE
  #define AN_ENCODE_VALUE(x) __builtin_an_encode_value_i64((x))
  #define AN_DECODE_VALUE(x) __builtin_an_decode_value_i64((x))
#else
  #define AN_ENCODE_VALUE(x) ((long long)x)
  #define AN_DECODE_VALUE(x) ((long long)x)
#endif

#endif /* __ENCODE_H__ */
