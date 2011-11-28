/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL: https://residual.svn.sourceforge.net/svnroot/residual/residual/trunk/tools/patchex/mszipd.cpp $
 * $Id: mszipd.cpp 1475 2009-06-18 14:12:27Z aquadran $
 *
 */

/* This file is part of libmspack.
 * (C) 2003-2004 Stuart Caie.
 *
 * This source code is adopted and striped for Residual project.
 *
 * The deflate method was created by Phil Katz. MSZIP is equivalent to the
 * deflate method.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */
/*
#include <cassert>
#include "tools/patchex/mszip.h"


static const unsigned short lit_lengths[29] = {
3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27,
31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};

static const unsigned short dist_offsets[30] = {
1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static const unsigned char lit_extrabits[29] = {
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2,
2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

static const unsigned char dist_extrabits[30] = {
0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

static const unsigned char bitlen_order[19] = {
16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static const unsigned short bit_mask[17] = {
0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define STORE_BITS do {                                                 \
_i_ptr      = i_ptr;                                              \
_i_end      = i_end;                                              \
_bit_buffer = bit_buffer;                                         \
_bits_left  = bits_left;                                          \
} while (0)

#define RESTORE_BITS do {                                               \
i_ptr      = _i_ptr;                                              \
i_end      = _i_end;                                              \
bit_buffer = _bit_buffer;                                         \
bits_left  = _bits_left;                                          \
} while (0)

#define ENSURE_BITS(nbits) do {                                         \
while (bits_left < (nbits)) {                                         \
if (i_ptr >= i_end) {                                               \
if (read_input()) return _error;                      \
i_ptr = _i_ptr;                                               \
i_end = _i_end;                                               \
}                                                                   \
bit_buffer |= *i_ptr++ << bits_left; bits_left  += 8;               \
}                                                                     \
} while (0)

#define PEEK_BITS(nbits)   (bit_buffer & ((1<<(nbits))-1))
#define PEEK_BITS_T(nbits) (bit_buffer & bit_mask[(nbits)])

#define REMOVE_BITS(nbits) ((bit_buffer >>= (nbits)), (bits_left -= (nbits)))

#define READ_BITS(val, nbits) do {                                      \
ENSURE_BITS(nbits); (val) = PEEK_BITS(nbits); REMOVE_BITS(nbits);     \
} while (0)

#define READ_BITS_T(val, nbits) do {                                    \
ENSURE_BITS(nbits); (val) = PEEK_BITS_T(nbits); REMOVE_BITS(nbits);   \
} while (0)

int mszipd_stream::read_input() {
	//printf ("inbuf_size %d\n",_inbuf_size);
	int read = _sys->read(_input, &_inbuf[0], (int)_inbuf_size);
	if (read < 0) return _error = MSPACK_ERR_READ;
	_i_ptr = &_inbuf[0];
	_i_end = &_inbuf[read];
	
	return MSPACK_ERR_OK;
}

int mszipd_stream::writeOutput(unsigned int amount) {
	unsigned int newSize = _outBufOffset + amount;
	if ((newSize) > _outBufSize) {
		newSize = newSize * 4;
		char *newBuf = new char[_outBufSize + newSize];
		memcpy(newBuf, _outBuf, _outBufSize);
		delete _outBuf;
		_outBuf = newBuf;
		_outBufSize = _outBufSize + newSize;
	}
	memcpy(_outBuf + _outBufOffset, _o_ptr, amount);
	_outBufOffset += amount;
	return amount;
}*/
/*
unsigned int mszipd_stream::writeFinalOutput() {
	return _sys->write(_output, _outBuf, _outBufOffset);
}*/
/*
#define INF_ERR_BLOCKTYPE   (-1)
#define INF_ERR_COMPLEMENT  (-2)
#define INF_ERR_FLUSH       (-3)
#define INF_ERR_BITBUF      (-4)
#define INF_ERR_SYMLENS     (-5)
#define INF_ERR_BITLENTBL   (-6)
#define INF_ERR_LITERALTBL  (-7)
#define INF_ERR_DISTANCETBL (-8)
#define INF_ERR_BITOVERRUN  (-9)
#define INF_ERR_BADBITLEN   (-10)
#define INF_ERR_LITCODE     (-11)
#define INF_ERR_DISTCODE    (-12)
#define INF_ERR_DISTANCE    (-13)
#define INF_ERR_HUFFSYM     (-14)

static int make_decode_table(unsigned int nsyms, unsigned int nbits, unsigned char *length, unsigned short *table)
{
	register unsigned int leaf, reverse, fill;
	register unsigned short sym, next_sym;
	register unsigned char bit_num;
	unsigned int pos         = 0;
	unsigned int table_mask  = 1 << nbits;
	unsigned int bit_mask    = table_mask >> 1;
	
	for (bit_num = 1; bit_num <= nbits; bit_num++) {
		for (sym = 0; sym < nsyms; sym++) {
			if (length[sym] != bit_num) continue;
			
			fill = length[sym]; reverse = pos >> (nbits - fill); leaf = 0;
			do {leaf <<= 1; leaf |= reverse & 1; reverse >>= 1;} while (--fill);
			
			if((pos += bit_mask) > table_mask) return 1;
			
			fill = bit_mask; next_sym = 1 << bit_num;
			do { table[leaf] = sym; leaf += next_sym; } while (--fill);
		}
		bit_mask >>= 1;
	}
	
	if (pos == table_mask) return 0;
	
	for (sym = pos; sym < table_mask; sym++) {
		reverse = sym; leaf = 0; fill = nbits;
		do { leaf <<= 1; leaf |= reverse & 1; reverse >>= 1; } while (--fill);
		table[leaf] = 0xFFFF;
	}
	
	next_sym = ((table_mask >> 1) < nsyms) ? nsyms : (table_mask >> 1);
	
	pos <<= 16;
	table_mask <<= 16;
	bit_mask = 1 << 15;
	
	for (bit_num = nbits+1; bit_num <= MSZIP_MAX_HUFFBITS; bit_num++) {
		for (sym = 0; sym < nsyms; sym++) {
			if (length[sym] != bit_num) continue;
			
			reverse = pos >> 16; leaf = 0; fill = nbits;
			do {leaf <<= 1; leaf |= reverse & 1; reverse >>= 1;} while (--fill);
			
			for (fill = 0; fill < (bit_num - nbits); fill++) {
				if (table[leaf] == 0xFFFF) {
					table[(next_sym << 1)     ] = 0xFFFF;
					table[(next_sym << 1) + 1 ] = 0xFFFF;
					table[leaf] = next_sym++;
				}
				leaf = (table[leaf] << 1) | ((pos >> (15 - fill)) & 1);
			}
			table[leaf] = sym;
			
			if ((pos += bit_mask) > table_mask) return 1;
		}
		bit_mask >>= 1;
	}
	
	return (pos != table_mask) ? 1 : 0;
}

#define READ_HUFFSYM(tbl, var) do {                                     \
ENSURE_BITS(MSZIP_MAX_HUFFBITS);                                      \
sym = _##tbl##_table[PEEK_BITS(MSZIP_##tbl##_TABLEBITS)];		\
if (sym >= MSZIP_##tbl##_MAXSYMBOLS) {                                \
i = MSZIP_##tbl##_TABLEBITS - 1;					\
do {                                                                \
if (i++ > MSZIP_MAX_HUFFBITS) {					\
return INF_ERR_HUFFSYM;                                         \
}                                                                 \
sym = _##tbl##_table[(sym << 1) | ((bit_buffer >> i) & 1)];	\
} while (sym >= MSZIP_##tbl##_MAXSYMBOLS);                          \
}                                                                     \
(var) = sym;                                                          \
i = _##tbl##_len[sym];                                              \
REMOVE_BITS(i);                                                       \
} while (0)

int mszipd_stream::read_lens() {
	register unsigned int bit_buffer;
	register int bits_left;
	unsigned char *i_ptr, *i_end;
	
	unsigned short bl_table[(1 << 7)];
	unsigned char bl_len[19];
	
	unsigned char lens[MSZIP_LITERAL_MAXSYMBOLS + MSZIP_DISTANCE_MAXSYMBOLS];
	unsigned int lit_codes, dist_codes, code, last_code=0, bitlen_codes, i, run;
	
	RESTORE_BITS;
	
	READ_BITS(lit_codes,    5); lit_codes    += 257;
	READ_BITS(dist_codes,   5); dist_codes   += 1;
	READ_BITS(bitlen_codes, 4); bitlen_codes += 4;
	if (lit_codes  > MSZIP_LITERAL_MAXSYMBOLS)  return INF_ERR_SYMLENS;
	if (dist_codes > MSZIP_DISTANCE_MAXSYMBOLS) return INF_ERR_SYMLENS;
	
	for (i = 0; i < bitlen_codes; i++) READ_BITS(bl_len[bitlen_order[i]], 3);
	while (i < 19) bl_len[bitlen_order[i++]] = 0;
	
	if (make_decode_table(19, 7, &bl_len[0], &bl_table[0])) {
		return INF_ERR_BITLENTBL;
	}
	
	for (i = 0; i < (lit_codes + dist_codes); i++) {
		ENSURE_BITS(7);
		code = bl_table[PEEK_BITS(7)];
		REMOVE_BITS(bl_len[code]);
		
		if (code < 16) lens[i] = last_code = code;
		else {
			switch (code) {
				case 16: READ_BITS(run, 2); run += 3;  code = last_code; break;
				case 17: READ_BITS(run, 3); run += 3;  code = 0;         break;
				case 18: READ_BITS(run, 7); run += 11; code = 0;         break;
				default: return INF_ERR_BADBITLEN;
			}
			if ((i + run) > (lit_codes + dist_codes)) return INF_ERR_BITOVERRUN;
			while (run--) lens[i++] = code;
			i--;
		}
	}
	
	i = lit_codes;
	memcpy(&_LITERAL_len[0], &lens[0], i);
	while (i < MSZIP_LITERAL_MAXSYMBOLS) _LITERAL_len[i++] = 0;
	
	i = dist_codes;
	memcpy(&_DISTANCE_len[0], &lens[lit_codes], i);
	while (i < MSZIP_DISTANCE_MAXSYMBOLS) _DISTANCE_len[i++] = 0;
	
	STORE_BITS;
	return 0;
}

int mszipd_stream::inflate() {
	unsigned int last_block, block_type, distance, length, this_run, i;
	
	register unsigned int bit_buffer;
	register int bits_left;
	register unsigned short sym;
	unsigned char *i_ptr, *i_end;

	RESTORE_BITS;

	do {
		READ_BITS(last_block, 1);
		
		READ_BITS(block_type, 2);
		
		if (block_type == 0) {
			unsigned char lens_buf[4];
			
			i = bits_left & 7; REMOVE_BITS(i);
			
			for (i = 0; (bits_left >= 8); i++) {
				if (i == 4) return INF_ERR_BITBUF;
				lens_buf[i] = PEEK_BITS(8);
				REMOVE_BITS(8);
			}
			if (bits_left != 0) return INF_ERR_BITBUF;
			while (i < 4) {
				if (i_ptr >= i_end) {
					if (read_input()) return _error;
					i_ptr = _i_ptr;
					i_end = _i_end;
				}
				lens_buf[i++] = *i_ptr++;
			}
			
			length = lens_buf[0] | (lens_buf[1] << 8);
			i      = lens_buf[2] | (lens_buf[3] << 8);
			if (length != (~i & 0xFFFF)) return INF_ERR_COMPLEMENT;
			
			while (length > 0) {
				if (i_ptr >= i_end) {
					if (read_input()) return _error;
					i_ptr = _i_ptr;
					i_end = _i_end;
				}
				
				this_run = length;
				if (this_run > (unsigned int)(i_end - i_ptr)) this_run = i_end - i_ptr;
				if (this_run > (MSZIP_FRAME_SIZE - _window_posn))
					this_run = MSZIP_FRAME_SIZE - _window_posn;
				
				memcpy(&_window[_window_posn], i_ptr, this_run);
				_window_posn += this_run;
				i_ptr    += this_run;
				length   -= this_run;
				
				if (_window_posn == MSZIP_FRAME_SIZE) {
					if (flush_window(MSZIP_FRAME_SIZE)) return INF_ERR_FLUSH;
					_window_posn = 0;
				}
			}
		}
		else if ((block_type == 1) || (block_type == 2)) {
			unsigned int window_posn, match_posn, code;
			
			if (block_type == 1) {
				i = 0;
				while (i < 144) _LITERAL_len[i++] = 8;
				while (i < 256) _LITERAL_len[i++] = 9;
				while (i < 280) _LITERAL_len[i++] = 7;
				while (i < 288) _LITERAL_len[i++] = 8;
				for (i = 0; i < 32; i++) _DISTANCE_len[i] = 5;
			}
			else {
				STORE_BITS;
				if ((i = read_lens())) return i;
				RESTORE_BITS;
			}
			
			if (make_decode_table(MSZIP_LITERAL_MAXSYMBOLS, MSZIP_LITERAL_TABLEBITS,
								  &_LITERAL_len[0], &_LITERAL_table[0]))
			{
				return INF_ERR_LITERALTBL;
			}
			
			if (make_decode_table(MSZIP_DISTANCE_MAXSYMBOLS,MSZIP_DISTANCE_TABLEBITS,
								  &_DISTANCE_len[0], &_DISTANCE_table[0]))
			{
				return INF_ERR_DISTANCETBL;
			}
			
			window_posn = _window_posn;
			while (1) {
				READ_HUFFSYM(LITERAL, code);
				if (code < 256) {
					_window[window_posn++] = (unsigned char) code;
					if (window_posn == MSZIP_FRAME_SIZE) {
						if (flush_window( MSZIP_FRAME_SIZE)) return INF_ERR_FLUSH;
						window_posn = 0;
					}
				}
				else if (code == 256) {
					break;
				}
				else {
					code -= 257;
					if (code > 29) return INF_ERR_LITCODE;
					READ_BITS_T(length, lit_extrabits[code]);
					length += lit_lengths[code];
					
					READ_HUFFSYM(DISTANCE, code);
					if (code > 30) return INF_ERR_DISTCODE;
					READ_BITS_T(distance, dist_extrabits[code]);
					distance += dist_offsets[code];
					
					match_posn = ((distance > window_posn) ? MSZIP_FRAME_SIZE : 0)
					+ window_posn - distance;
					
					if (length < 12) {
						while (length--) {
							_window[window_posn++] = _window[match_posn++];
							match_posn &= MSZIP_FRAME_SIZE - 1;
							
							if (window_posn == MSZIP_FRAME_SIZE) {
								if (flush_window( MSZIP_FRAME_SIZE))
									return INF_ERR_FLUSH;
								window_posn = 0;
							}
						}
					}
					else {
						unsigned char *runsrc, *rundest;
						do {
							this_run = length;
							if ((match_posn + this_run) > MSZIP_FRAME_SIZE)
								this_run = MSZIP_FRAME_SIZE - match_posn;
							if ((window_posn + this_run) > MSZIP_FRAME_SIZE)
								this_run = MSZIP_FRAME_SIZE - window_posn;
							
							rundest = &_window[window_posn]; window_posn += this_run;
							runsrc  = &_window[match_posn];  match_posn  += this_run;
							length -= this_run;
							while (this_run--) *rundest++ = *runsrc++;
							
							if (window_posn == MSZIP_FRAME_SIZE) {
								if (flush_window( MSZIP_FRAME_SIZE))
									return INF_ERR_FLUSH;
								window_posn = 0;
							}
							if (match_posn == MSZIP_FRAME_SIZE) match_posn = 0;
						} while (length > 0);
					}
					
				}
				
			}
			_window_posn = window_posn;
		}
		else {
			return INF_ERR_BLOCKTYPE;
		}
	} while (!last_block);
	
	if (_window_posn) {
		if (flush_window( _window_posn)) return INF_ERR_FLUSH;
	}
	STORE_BITS;
	
	return 0;
}

int mszipd_stream::flush_window(unsigned int data_flushed)
{
	_bytes_output += data_flushed;
	if (_bytes_output > MSZIP_FRAME_SIZE) {
		return 1;
	}
	return 0;
}

mszipd_stream::mszipd_stream(struct dec_system *system, PackFile *input, PackFile *output)
{
	assert(system);
	
	const int PARAM_FIXMSZIP = (0);
	const int PARAM_DECOMPBUF = (65536);
	
	int input_buffer_size = (PARAM_DECOMPBUF + 1) & -2;
	assert(input_buffer_size);
	
	_inbuf  = (unsigned char *)new size_t[input_buffer_size];
	assert(_inbuf);
	
	_sys             = system;
	_input           = input;
	_output          = output;
	_inbuf_size      = input_buffer_size;
	_error           = MSPACK_ERR_OK;
	_repair_mode     = PARAM_FIXMSZIP;
	
	_i_ptr = _i_end = &_inbuf[0];
	_o_ptr = _o_end = NULL;
	_bit_buffer = 0; _bits_left = 0;
	
	_outBuf = new char[512];
	_outBufOffset = 0;
	_outBufSize = 512;
}

int mszipd_stream::decompress(off_t out_bytes) {
	register unsigned int bit_buffer;
	register int bits_left;
	unsigned char *i_ptr, *i_end;
	
	if (_outBufOffset)
		_outBufOffset = 0;
	
	int i, state, error;
	
	if (out_bytes < 0) return MSPACK_ERR_ARGS;
	if (_error) return _error;
	
	i = _o_end - _o_ptr;
	if ((off_t) i > out_bytes) i = (int) out_bytes;
	if (i) {
		if (writeOutput(i) != i) {
			return error = MSPACK_ERR_WRITE;
		}
		_o_ptr  += i;
		out_bytes   -= i;
	}
	if (out_bytes == 0) return MSPACK_ERR_OK;
	
	
	while (out_bytes > 0) {
		RESTORE_BITS;
		i = bits_left & 7; REMOVE_BITS(i);
		state = 0;
		do {
			READ_BITS(i, 8);
			if (i == 'C') state = 1;
			else if ((state == 1) && (i == 'K')) state = 2;
			else state = 0;
		} while (state != 2);
		
		_window_posn = 0;
		_bytes_output = 0;
		STORE_BITS;
		if (_error = inflate()) {
			if (_repair_mode) {
				fprintf(stderr, "MSZIP error, %u bytes of data lost.",
						MSZIP_FRAME_SIZE - _bytes_output);
				for (i = _bytes_output; i < MSZIP_FRAME_SIZE; i++) {
					_window[i] = '\0';
				}
				_bytes_output = MSZIP_FRAME_SIZE;
			}
			else {
				return _error = (_error > 0) ? _error : MSPACK_ERR_DECRUNCH;
			}
		}
		_o_ptr = &_window[0];
		_o_end = &_o_ptr[_bytes_output];
		
		i = (out_bytes < (off_t)_bytes_output) ?
		(int)out_bytes : _bytes_output;
		if (writeOutput(i) != i) {
			return _error = MSPACK_ERR_WRITE;
		}
		
		if ((_error > 0) && _repair_mode) return _error;
		
		_o_ptr  += i;
		out_bytes   -= i;
	}
	
	if (out_bytes) {
		return error = MSPACK_ERR_DECRUNCH;
	}*/
/*	if (writeFinalOutput() != _outBufOffset) {
		return MSPACK_ERR_WRITE;
	}*/
/*	return MSPACK_ERR_OK;
}

mszipd_stream::~mszipd_stream() {
	if (_inbuf) {
		delete _inbuf;
	}
	if (_outBuf) {
		delete _outBuf;
	}
}
*/