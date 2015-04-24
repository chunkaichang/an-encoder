/*
 *  S-DES file encryption program
 *
 * Copyright (c) 2009, AlferSoft (www.alfersoft.com.ar - fvicente@gmail.com)
 * All rights reserved.
 * 
 * BSD License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY AlferSoft ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AlferSoft BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define USAGE	"Usage: sdes <-e | -d> <input file> <output file> <key 0-1023>\n"

extern void ___enc_generate_sub_keys(long, long*, long*);
extern long ___enc_des(long, long, long);


void display_bits(long num, long bits) {
	long i;

	if(bits < 1 || bits > 32) {
		printf("Invalid bit quantity");
		return;
	}
	/* display binary representation */
	for(i = 1L << (bits-1); i > 0L; i >>= 1L) {
		printf("%s",(i & num) ? "1" : "0");
	}
	printf("\n");
}


/**
 * Main program entry point
 */
int main(int argc, char* argv[]) {
        uint64_t        t1, t2, total = 0;
        unsigned        i, j;
	FILE		*in,*out;
	short int	key;
	long		ch = 0, sk1 = 0, sk2 = 0;

	if (argc < 5) {
		printf(USAGE);
		return(1);
	}
	key = atoi(argv[4]);
	if ((key < 0 || key > 1023) || strlen(argv[1]) != 2 || argv[1][0] != '-') {
		printf(USAGE);
		return(1);
	}
	key = (key % 1024);
	switch (argv[1][1]) {
		case 'e':		// encrypt
			___enc_generate_sub_keys(key,&sk1,&sk2);
			break;
		case 'd':		// decrypt
			___enc_generate_sub_keys(key,&sk2,&sk1);
			break;
		default:
			printf(USAGE);
			return(1);
	}
        printf("before: sk1=%lX, sk2=%lX\n", sk1, sk2);
        sk1 = AN_DECODE_VALUE(sk1);
        sk2 = AN_DECODE_VALUE(sk2);
        printf("after: sk1=%lX, sk2=%lX\n", sk1, sk2);

	// open input and output files
	in = fopen(argv[2], "rb");
	if (!in) {
		printf("File not found %s\n", argv[2]);
		return(1);
	}
	out = fopen(argv[3], "wb+");
	if (!out) {
		printf("Error creating output file %s\n", argv[3]);
		fclose(in);
		return(1);
	}

	printf("Processing...\n");
        __cs_fopen(argc, argv);
        __cs_reset();
	while (!feof(in)) {
		ch = (long)fgetc(in);

                t1 = __cyc_rdtsc();
                ch = ___enc_des(ch, sk1, sk2);
                t2 = __cyc_rdtsc();
                total += t2 - t1;

                __cs_facc(ch);
                __cs_acc(ch);
		fputc((char)ch, out);
	}
	printf("Ready!\n");
	fclose(in);
	fclose(out);

        __cyc_msg(total);
        __cs_fclose();
        __cs_msg();

	return 0;
}
