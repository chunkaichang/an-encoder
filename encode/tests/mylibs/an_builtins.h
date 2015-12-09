#ifndef __AN_BUILTINS_H__
#define __AN_BUILTINS_H__

#include <stdint.h>

int64_t an_encode(int64_t, int64_t);
int64_t an_decode(int64_t, int64_t);
int64_t an_check (int64_t, int64_t);
void    an_assert(int64_t, int64_t);

int64_t an_encode_value(int64_t);
int64_t an_decode_value(int64_t);
int64_t an_check_value (int64_t);
void    an_assert_value(int64_t);

#endif /* __AN_BUILTINS_H__ */
