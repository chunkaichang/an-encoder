
#include <stdio.h>

#define UPDC32(octet,crc) (crc_32_tab[((crc)^(octet)) & 0xff]^ ((crc) >> 8))

extern long crc_32_tab[256];

long crc32file(long *input, long *crc, long *charcnt)
{
    register long oldcrc32;
    register long tmp;
    register long c;

    oldcrc32 = ~0;
    *charcnt = 0;

    while (*input != EOF)
    {
        ++*charcnt;
        tmp = oldcrc32;
        oldcrc32 = UPDC32(*input, tmp);
        ++input;
    }

    tmp = ~oldcrc32;
    oldcrc32 = tmp;
    *crc = oldcrc32;


    return 0;
}

long ___enc_computation(long *input, long *crc, long *charcnt)
{
    long r = crc32file(input, crc, charcnt);

    return r;
}
