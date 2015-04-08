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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define DBG(desc, num, bits)		dbg_num(desc, num, 32);
//#define DBG(desc, num, bits)		dbg_num(desc, num, bits);
#define DBG(desc, num, bits)		//

static const long BIT_1 = 1;
static const long BIT_2 = 2;
static const long BIT_3 = 4;
static const long BIT_4 = 8;
static const long BIT_5 = 16;
static const long BIT_6 = 32;
static const long BIT_7 = 64;
static const long BIT_8 = 128;
static const long BIT_9 = 256;
static const long BIT_10 = 512;

static const long BITMASK_RIGHT = 1 | 2 | 4 | 8;

extern void display_bits(long num, long bits);

void dbg_num(char *desc, long num, long bits) {
	printf("%s", desc);
	display_bits(num, bits);
}

long sbox0[][4] = {
			1, 0, 3, 2,
			3, 2, 1, 0,
			0, 2, 1, 3,
			3, 1, 3, 2
		                  };
long sbox1[][4] = {
			0, 1, 2, 3,
			2, 0, 1, 3,
			3, 0, 1, 0,
			2, 1, 0, 3
				  };


/**
 * Generate two 8-bit subkeys based on a 10-bit key
 *
 * @param key  10-bit key
 * @param sk1  pointer that receives the first subkey
 * @param sk2  pointer that receives the second subkey
 */
void ___enc_generate_sub_keys(long key, long *sk1, long *sk2) {
	long i;
	long k1_order[] = { BIT_10, BIT_4, BIT_2, BIT_7, BIT_3, BIT_8, BIT_1, BIT_5 };
	long k2_order[] = { BIT_3, BIT_8, BIT_5, BIT_6, BIT_1, BIT_9, BIT_2, BIT_10 };

	DBG("key: ", key, 8);
	*sk1 = 0;
	*sk2 = 0;
	for (i = 0; i < 8; i++) {
		*sk1 = (*sk1 << 1) | !!(k1_order[i] & key);
		*sk2 = (*sk2 << 1) | !!(k2_order[i] & key);
		DBG("(k1_order[i] & key) ", (k1_order[i] & key), 8);
		DBG("!(k1_order[i] & key) ", !(k1_order[i] & key), 8);
		DBG("!!(k1_order[i] & key) ", !!(k1_order[i] & key), 8);
	        DBG("sk1: ", *sk1, 8);
	}
}

/**
 * F function used by Fk algorithm. Input are the 4-bits to convert and a subkey.
 * It performs an expansion/permutation, xor with subkey and sbox mapping + permutation
 *
 * @param input  4-bits
 * @param sk  subkey to combine
 */
long f(long input, long sk) {
	long row0=0,col0=0,row1=0,col1=0;
	long out=0,ep=0,aux=0;

	DBG("f() input: ", (int)input, 8);

	// E/P
	aux = !!(input & BIT_1);
	ep |= ((aux << 7L) | (aux << 1L));
	aux = !!(input & BIT_2);
	ep |= ((aux << 4L) | (aux << 2L));
	aux = !!(input & BIT_3);
	ep |= ((aux << 5L) | (aux << 3L));
	aux = !!(input & BIT_4);
	ep |= ((aux << 6L) | aux);
	DBG("E/P: ", (int)ep, 8L);

	DBG("sk: ", sk, 8L);
	// xor with subkey
	ep = ep ^ sk;
	DBG("E/P ^ sk: ", ep, 8L);

	// calculate row and columns for sboxes
	row0 = !!(ep & BIT_8);
	row0 = (row0 << 1L) | !!(ep & BIT_5);
	col0 = !!(ep & BIT_7);
	col0 = (col0 << 1L) | !!(ep & BIT_6);
	row1 = !!(ep & BIT_4);
	row1 = (row1 << 1L) | !!(ep & BIT_1);
	col1 = !!(ep & BIT_3);
	col1 = (col1 << 1L) | !!(ep & BIT_2);

	// P4 (2,4,3,1)
	out = !!(sbox0[row0][col0] & BIT_1);
	out = (out << 1L) | !!(sbox1[row1][col1] & BIT_1);
	out = (out << 1L) | !!(sbox1[row1][col1] & BIT_2);
	out = (out << 1L) | !!(sbox0[row0][col0] & BIT_2);
	DBG("f() out: ", (long)(out), 8L);

	return(out);
}

/**
 * Main Fk complex algorithm
 *
 * @param input  byte to apply Fk algorithm
 * @param sk1  first subkey
 * @param sk2  second subkey
 */
long ___enc_fk(long input, long sk1, long sk2) {
	long l=0,r=0,out=0;

	DBG("Fk sk1: ", sk1, 8);
	DBG("Fk sk2: ", sk2, 8);
	DBG("Fk IN: ", input, 8);
	// first 4 bits
	r = (input & BITMASK_RIGHT);
	l = (input >> 4);
	out = ((f(r, sk1) ^ l) & BITMASK_RIGHT) | (r << 4);		// the output is switched here
	DBG("Fk OUT + Swap: ", out, 8);
	// second 4 bits
	r = (out & BITMASK_RIGHT);
	l = (out >> 4);
	out = ((f(r, sk2) ^ l) << 4) | r;
	DBG("Fk OUT: ", out, 8);
	return(out);
}

/**
 * Initial permutation
 *
 * @param byte  input to apply the permutation
 */
long ___enc_ip(long byte) {
	long i;
	long ret;
	long order[] = { BIT_7, BIT_3, BIT_6, BIT_8, BIT_5, BIT_1, BIT_4, BIT_2 };

	DBG("IP IN: ", byte, 8);
	ret = 0;
	for (i = 0; i < 8; i++) {
		ret = (ret << 1) | !!(order[i] & byte);
		DBG("IP (order[i] & byte) ", (order[i] & byte), 8);
		DBG("IP !(order[i] & byte) ", !(order[i] & byte), 8);
		DBG("IP !!(order[i] & byte) ", !!(order[i] & byte), 8);
	        DBG("IP: ", ret, 8);
	}
	DBG("IP OUT: ", ret, 8);
	return(ret);
}

/**
 * Initial permutation inverted
 *
 * @param byte  input to apply the inverted permutation
 */
long ___enc_ip_inverse(long byte) {
	long i;
	long ret;
	long order[] = { BIT_5, BIT_8, BIT_6, BIT_4, BIT_2, BIT_7, BIT_1, BIT_3 };

	ret = 0;
	for (i = 0; i < 8; i++) {
		ret = (ret << 1) | !!(order[i] & byte);
	}
	return(ret);
}
