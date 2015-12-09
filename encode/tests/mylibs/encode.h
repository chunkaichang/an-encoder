#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "an_builtins.h"

#ifdef ENCODE
  #define AN_ENCODE_VALUE(x) an_encode_value((x))
  #define AN_DECODE_VALUE(x) an_decode_value((x))
#else
  #define AN_ENCODE_VALUE(x) ((long long)x)
  #define AN_DECODE_VALUE(x) ((long long)x)
#endif

#endif /* __ENCODE_H__ */
