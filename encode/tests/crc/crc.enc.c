
#include <stdio.h>

#define UPDC32(octet,crc) (crc_32_tab[((crc)^(octet)) & 0xff]^ ((crc) >> 8))

extern long crc_32_tab[256];
extern long mygetc(FILE *);

long crc32file(FILE *fin, long *crc, long *charcnt)
{
    register long oldcrc32;
    register long tmp;
    register long c;

    oldcrc32 = ~0;
    *charcnt = 0;

    c = mygetc(fin);
    while (c != EOF)
    {
        ++*charcnt;
        tmp = oldcrc32;
        oldcrc32 = UPDC32(c, tmp);
        c = mygetc(fin);
    }

    tmp = ~oldcrc32;
    oldcrc32 = tmp;
    *crc = oldcrc32;


    return 0;
}

long ___enc_computation(FILE *fin, long *crc, long *charcnt)
{
    long r = crc32file(fin, crc, charcnt);

    return r;
}
