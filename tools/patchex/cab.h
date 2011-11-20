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
#include "tools/patchex/mszip.h"

#define cfhead_Signature         (0x00)
#define cfhead_CabinetSize       (0x08)
#define cfhead_FileOffset        (0x10)
#define cfhead_MinorVersion      (0x18)
#define cfhead_MajorVersion      (0x19)
#define cfhead_NumFolders        (0x1A)
#define cfhead_NumFiles          (0x1C)
#define cfhead_Flags             (0x1E)
#define cfhead_SetID             (0x20)
#define cfhead_CabinetIndex      (0x22)
#define cfhead_SIZEOF            (0x24)
#define cfheadext_HeaderReserved (0x00)
#define cfheadext_FolderReserved (0x02)
#define cfheadext_DataReserved   (0x03)
#define cfheadext_SIZEOF         (0x04)
#define cffold_DataOffset        (0x00)
#define cffold_NumBlocks         (0x04)
#define cffold_CompType          (0x06)
#define cffold_SIZEOF            (0x08)
#define cffile_UncompressedSize  (0x00)
#define cffile_FolderOffset      (0x04)
#define cffile_FolderIndex       (0x08)
#define cffile_Date              (0x0A)
#define cffile_Time              (0x0C)
#define cffile_Attribs           (0x0E)
#define cffile_SIZEOF            (0x10)
#define cfdata_CheckSum          (0x00)
#define cfdata_CompressedSize    (0x04)
#define cfdata_UncompressedSize  (0x06)
#define cfdata_SIZEOF            (0x08)

#define cffoldCOMPTYPE_MASK            (0x000f)
#define cffoldCOMPTYPE_NONE            (0x0000)
#define cffoldCOMPTYPE_MSZIP           (0x0001)
#define cfheadPREV_CABINET             (0x0001)
#define cfheadNEXT_CABINET             (0x0002)
#define cfheadRESERVE_PRESENT          (0x0004)
#define cffileCONTINUED_FROM_PREV      (0xFFFD)
#define cffileCONTINUED_TO_NEXT        (0xFFFE)
#define cffileCONTINUED_PREV_AND_NEXT  (0xFFFF)

#define CAB_BLOCKMAX (32768)
#define CAB_INPUTMAX (CAB_BLOCKMAX+6144)

struct mspack_system;
struct mszipd_stream;
struct mscabd_folder;
struct mscabd_folder_data;
class mscabd_decompress_state;

class mscabd_decompress_state {
public:
	mscabd_decompress_state();
	~mscabd_decompress_state();
	int init(PackFile * fh, unsigned int ct);
	mscabd_folder *folder;
	mscabd_folder_data *data;
	unsigned int offset;
	unsigned int block;
	int comp_type;
	int decompress(off_t offset) { return ZipDecompress(offset); }
	int ZipDecompress(off_t offset);
	mszipd_stream *state;
	struct mscabd_cabinet *incab;
	struct PackFile *infh;
	struct PackFile *outfh;
	unsigned char *i_ptr, *i_end;
	unsigned char input[CAB_INPUTMAX];
private:
	struct PackFile *initfh;
	dec_system decsys;
};

char *file_filter(const struct mscabd_file *file);

struct mscabd_cabinet {
	mscabd_cabinet(std::string fname);
	int read_headers(off_t offset, int quiet);
	std::string read_string(int *error);
	int read_block(int *out, int ignore_cksum);
	std::string GetFilename() { return filename; }
	mscabd_cabinet *next;
	
	off_t base_offset;
	unsigned int length;
	mscabd_cabinet *prevcab;
	mscabd_cabinet *nextcab;
	std::string prevname, nextname, previnfo, nextinfo;
	struct mscabd_file *files;
	struct mscabd_folder *folders;
	int block_resv;
private:
	unsigned short set_id;
	unsigned short set_index;
	unsigned short header_resv;
	int flags;
	off_t blocks_off;
	std::string filename;
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
	
	unsigned int getCabLength() { return _cab->length; }
//private:
	
	

//	int param[3];
	int error;
	unsigned int lang;
private:
		struct mscabd_decompress_state *d;
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
	struct mscabd_folder *next;
	int comp_type;
	unsigned int num_blocks;
	
	mscabd_folder_data data;
	struct mscabd_file *merge_prev;
	struct mscabd_file *merge_next;
};

#endif
