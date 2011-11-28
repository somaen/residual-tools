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
 * $URL: https://residual.svn.sourceforge.net/svnroot/residual/residual/trunk/tools/patchex/mszip.h $
 * $Id: mszip.h 1475 2009-06-18 14:12:27Z aquadran $
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

#ifndef MSPACK_MSZIP_H
#define MSPACK_MSZIP_H 1


#include <cassert>
/*

#define MSZIP_FRAME_SIZE          (32768)
#define MSZIP_MAX_HUFFBITS        (16)
#define MSZIP_LITERAL_MAXSYMBOLS  (288)
#define MSZIP_LITERAL_TABLEBITS   (9)
#define MSZIP_DISTANCE_MAXSYMBOLS (32)
#define MSZIP_DISTANCE_TABLEBITS  (6)

#if (1 << MSZIP_LITERAL_TABLEBITS) < (MSZIP_LITERAL_MAXSYMBOLS * 2)
# define MSZIP_LITERAL_TABLESIZE (MSZIP_LITERAL_MAXSYMBOLS * 4)
#else
# define MSZIP_LITERAL_TABLESIZE ((1 << MSZIP_LITERAL_TABLEBITS) + \
(MSZIP_LITERAL_MAXSYMBOLS * 2))
#endif

#if (1 << MSZIP_DISTANCE_TABLEBITS) < (MSZIP_DISTANCE_MAXSYMBOLS * 2)
# define MSZIP_DISTANCE_TABLESIZE (MSZIP_DISTANCE_MAXSYMBOLS * 4)
#else
# define MSZIP_DISTANCE_TABLESIZE ((1 << MSZIP_DISTANCE_TABLEBITS) + \
(MSZIP_DISTANCE_MAXSYMBOLS * 2))
#endif

class mszipd_stream {
public:
	mszipd_stream(dec_system *system, PackFile *input, PackFile *output);
	~mszipd_stream();
	int decompress(off_t out_bytes);
	char *getData() { return _outBuf; } // Very thread-unsafe
	unsigned int getLen() { return _outBufOffset; }
private:
	dec_system *_sys;
	
	PackFile   *_input;
	PackFile   *_output;
	
	int flush_window(unsigned int);
	
	unsigned int _window_posn;
	int _error, _repair_mode, _bytes_output;
	
	unsigned char *_inbuf, *_i_ptr, *_i_end, *_o_ptr, *_o_end;
	unsigned int _bit_buffer, _bits_left, _inbuf_size;
	
	unsigned char  _LITERAL_len[MSZIP_LITERAL_MAXSYMBOLS];
	unsigned char  _DISTANCE_len[MSZIP_DISTANCE_MAXSYMBOLS];
	
	unsigned short _LITERAL_table [MSZIP_LITERAL_TABLESIZE];
	unsigned short _DISTANCE_table[MSZIP_DISTANCE_TABLESIZE];
	
	unsigned char _window[MSZIP_FRAME_SIZE];
	
	unsigned int _outBufOffset, _outBufSize;
	char *_outBuf;
	
	int read_lens();
	int read_input();
	int writeOutput(unsigned int);
//	unsigned int writeFinalOutput();
	int inflate();
};
*/
#endif
 
