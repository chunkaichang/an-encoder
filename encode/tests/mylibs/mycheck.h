#ifndef __MYCHECK_H__
#define __MYCHECK_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

__uint128_t __cs_reset();
__uint128_t __cs_get();
__uint128_t __cs_acc(__uint128_t x);
void        __cs_msg(void);

void        __cs_fopen(int argc, char** argv);
size_t      __cs_facc(__uint128_t x);
void        __cs_fclose();

FILE*       __cs_log(int argc, char** argv);

#endif /* __MYCHECK_H__ */
