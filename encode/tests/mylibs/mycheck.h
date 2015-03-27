#ifndef __MYCHECK_H__
#define __MYCHECK_H__

#include <stdint.h>

__uint128_t __cs_reset();
__uint128_t __cs_get();
__uint128_t __cs_acc(__uint128_t x);
void        __cs_msg(void);

#endif /* __MYCHECK_H__ */
