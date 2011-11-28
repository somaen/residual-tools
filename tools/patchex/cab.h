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
 * $URL $
 * $Id $
 *
 */

/* This file is part of libmspack.
 * (C) 2003-2004 Stuart Caie.
 *
 * This source code is adopted and striped for Residual project.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */

#ifndef MSPACK_CAB_H
#define MSPACK_CAB_H

#include <string>
#include <zlib.h>
#include "tools/patchex/mspack.h"
#include "tools/patchex/packfile.h"

const int cfheadext_HeaderReserved	= (0x00);
const int cfheadext_FolderReserved	= (0x02);
const int cfheadext_DataReserved	= (0x03);
const int cfheadext_SIZEOF			= (0x04);

const int cfdata_CheckSum			= (0x00);
const int cfdata_CompressedSize		= (0x04);
const int cfdata_UncompressedSize	= (0x06);
const int cfdata_SIZEOF				= (0x08);

const unsigned int cffoldCOMPTYPE_MASK		= (0x000f);
const unsigned int cffoldCOMPTYPE_NONE		= (0x0000);
const unsigned int cffoldCOMPTYPE_MSZIP		= (0x0001);
const unsigned int cfheadPREV_CABINET		= (0x0001);
const unsigned int cfheadNEXT_CABINET		= (0x0002);
const unsigned int cfheadRESERVE_PRESENT	= (0x0004);

#define CAB_BLOCKMAX (32768)
#define CAB_INPUTMAX (CAB_BLOCKMAX+6144)

//struct mspack_system;
struct mscabd_folder;
struct mscabd_folder_data;
class mscabd_decompress_state;

class mscabd_decompress_state {
public:
	mscabd_decompress_state();
	int init(PackFile * fh, unsigned int ct);
	
	mscabd_folder *_folder;
	mscabd_folder_data *_data;
	unsigned int _offset;
	unsigned int _block;
	int _comp_type;
	
	int decompress(off_t preread, off_t offset, off_t length) { return ZipDecompress(preread, offset, length); }
	int ZipDecompress(off_t preread, off_t offset, off_t length);
	int	zlibDecompress(off_t preread, off_t offset, off_t length, Bytef *&ret);
	struct mscabd_cabinet *_incab;
	struct PackFile *_infh;
	struct PackFile *_outfh;

	unsigned char *_i_ptr, *_i_end;
	unsigned char _input[CAB_INPUTMAX];
private:
	int write(struct PackFile *file, void *buffer, int bytes);
	struct PackFile *_initfh;
	//dec_system _decsys;
};

char *file_filter(const struct mscabd_file *file);

struct mscabd_cabinet {
	mscabd_cabinet(std::string fname);
	int read_headers(off_t offset, int quiet);
	int read_block(int *out, int ignore_cksum);
	std::string GetFilename() { return _filename; }
	mscabd_cabinet *_next;
	
	off_t _base_offset;
	unsigned int _length;
	mscabd_cabinet *_prevcab;
	mscabd_cabinet *_nextcab;
	std::string _prevname, _nextname, _previnfo, _nextinfo;
	struct mscabd_file *_files;
	struct mscabd_folder *_folders;
	int _block_resv;
private:
	unsigned short _set_id;
	unsigned short _set_index;
	unsigned short _header_resv;
	int _flags;
	off_t _blocks_off;
	std::string _filename;
	PackFile *_fh;
};

struct mscab_decompressor {
	mscab_decompressor();
	~mscab_decompressor();
	void open(std::string filename);
	void close();
	void printFiles();
	void extract_files();
	
	int extract(struct mscabd_file *file, std::string filename);
	int last_error();
	
	unsigned int getCabLength() { return _cab->_length; }

	unsigned int lang;
private:
	
	struct mscabd_decompress_state *d;
	int error;

private:
		
	int init_decomp(unsigned int ct);
	void free_decomp();
	mscabd_cabinet *_cab;
	std::string read_string(PackFile *fh, mscabd_cabinet *, int *error);
	std::string file_filter(const struct mscabd_file *file);
};

struct mscabd_folder_data {
	mscabd_folder_data *next;
	mscabd_cabinet *cab;
	off_t offset;
};

struct mscabd_folder {
	mscabd_folder(unsigned char buf[64]);
	struct mscabd_folder *_next;
	int _comp_type;
	unsigned int _num_blocks;
	
	mscabd_folder_data _data;
	struct mscabd_file *_merge_prev;
	struct mscabd_file *_merge_next;
};

#endif
