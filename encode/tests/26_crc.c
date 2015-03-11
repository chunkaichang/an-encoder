/**********************************************************************\
|* Demonstration program to compute the 32-bit CRC used as the frame  *|
|* check sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71     *|
|* and FED-STD-1003, the U.S. versions of CCITT's X.25 link-level     *|
|* protocol).  The 32-bit FCS was added via the Federal Register,     *|
|* 1 June 1982, p.23798.  I presume but don't know for certain that   *|
|* this polynomial is or will be included in CCITT V.41, which        *|
|* defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS  *|
|* PUB 78 says that the 32-bit FCS reduces otherwise undetected       *|
|* errors by a factor of 10^-5 over 16-bit FCS.                       *|
\**********************************************************************/

/* Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.*/

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*  1. The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*  2. The CRC accumulation logic is the same for all CRC polynomials, */
/*     be they sixteen or thirty-two bits wide.  You simply choose the */
/*     appropriate table.  Alternatively, because the table can be     */
/*     generated at runtime, you can start by generating the table for */
/*     the polynomial in question and use exactly the same "updcrc",   */
/*     if your application needn't simultaneously handle two CRC       */
/*     polynomials.  (Note, however, that XMODEM is strange.)          */
/*                                                                     */
/*  3. For 16-bit CRCs, the table entries need be only 16 bits wide;   */
/*     of course, 32-bit entries work OK if the high 16 bits are zero. */
/*                                                                     */
/*  4. The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long  BYTE;
typedef unsigned long long  DWORD;
typedef unsigned short WORD;
typedef DWORD UNS_32_BITS;

extern UNS_32_BITS crc_32_tab(BYTE);

#define NUL '\0'
#define UPDC32(octet,crc) (crc_32_tab(((crc)^((BYTE)octet)) & 0xff) ^ ((crc) >> 8))

#define log2(...) fprintf(stdout, __VA_ARGS__)

DWORD updateCRC32(unsigned long long ch, DWORD crc);
DWORD crc32buf(char *buf, size_t len);

#ifdef __TURBOC__
 #pragma warn -cln
#endif



DWORD updateCRC32(unsigned long long ch, DWORD crc)
{
      return UPDC32(ch, crc);
}

DWORD crc32buf(char *buf, size_t len)
{
    register DWORD oldcrc32;
    oldcrc32 = 0xFFFFFFFF;

    for ( ; len > 0; --len)
    {
        oldcrc32 = UPDC32(*buf, oldcrc32);
        ++buf;
    }

    return ~oldcrc32;
}

int crc32file(FILE *fin, DWORD *crc, unsigned long long *charcnt)
{
    register unsigned long long oldcrc32;
    register unsigned long long tmp;
    register unsigned long long c;

    oldcrc32 = 0xFFFFFFFF;
    *charcnt = 0;

    c = getc(fin);
    while (c != EOF)
    {
        ++*charcnt;
        tmp = oldcrc32;
        oldcrc32 = UPDC32(c, tmp);
        c = getc(fin);
    }

    tmp = ~oldcrc32;
    oldcrc32 = tmp;
    *crc = oldcrc32;


    return 0;
}

void precomputation() {}
void postcomputation() {}

/*
 * computation() is the main entry for encoding.
 * Python translator should:
 *   -- extract the AST-subtree upon finding computation()
 *   -- transform code in this subtree using one of the encodings
 *       -- encode all args
 *       -- encode all internal functions
 *       -- encode all constants
 *       -- decode and check return value
 *
 */

int computation(FILE *fin)
{
    DWORD crc;
    unsigned long long charcnt;
    int r = crc32file(fin, &crc, &charcnt);

    log2("%16llX %7lld\n", crc, charcnt);

    return r;
}

int main(int argc, char *argv[])
{
// correct output - FFFFFFFF6DA5B639 1368864
    //if (argc != 2)
    //    return -1;

    int errors = 0;
    FILE *fin;

    //fin = fopen(argv[1], "r");
    fin = fopen("tests/inputs/crc_input_small.pcm", "r");
    if (fin == NULL)
    {
        perror(argv[1]);
        return -1;
    }

    precomputation();
    errors |= computation(fin);
    postcomputation();

    if (ferror(fin))
        perror(argv[1]);

    fclose(fin);

    return(errors != 0);
}
