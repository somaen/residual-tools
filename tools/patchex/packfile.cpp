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
 * $URL: https://residual.svn.sourceforge.net/svnroot/residual/residual/trunk/tools/patchex/cabd.cpp $
 * $Id: cabd.cpp 1475 2009-06-18 14:12:27Z aquadran $
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
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include "packfile.h"


// Extraction constans
#define RAND_A				(0x343FD)
#define RAND_B				(0x269EC3)
#define CODE_TABLE_SIZE		(0x100)
#define CONTAINER_MAGIC		"1CNT"
#define CABINET_MAGIC		"MSCF"

uint32 READ_LE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
}

// Res_system
uint16 *PackFile::create_dec_table(uint32 key) {
	uint32 value;
	uint16 *dectable;
	unsigned int i;
	
	value = key;
	dectable = new uint16[CODE_TABLE_SIZE * 2];
	
	for (i = 0; i < CODE_TABLE_SIZE; i++) {
		value = RAND_A * value + RAND_B;
		dectable[i] = (uint16)((value >> 16) & 0x7FFF);
	}
	
	return dectable;
}

void PackFile::decode(uint8 *data, unsigned int size, uint16 *dectable, unsigned int start_point) {
	unsigned int i;
	for (i = 0; i < size; i++)
		data[i] = (data[i] ^ (uint8) dectable[(i + start_point) % CODE_TABLE_SIZE]) - (uint8)(dectable[(i + start_point) % CODE_TABLE_SIZE] >> 8);
}

PackFile::PackFile(std::string filename, int mode) {
	//printf("Call to open %s with mode %d\n",filename.c_str(), mode);
	
	const char *fmode;
	uint32 magic, key;
	uint8 count;
	
	switch (mode) {
		case OPEN_READ:   fmode = "rb";  break;
		case OPEN_WRITE:  fmode = "wb";  break;
		default: 
			printf("Wrong mode\n");
			assert(0);
	}
	
	this->name = filename;
	if (!(this->fh = fopen(filename.c_str(), fmode))) {
		delete fh;
		assert(0); // TODO FIX BETTER SOLUTION
	}
	
	this->CodeTable = NULL;
	
	if (mode == OPEN_READ) {
		
		//Search for data
		while(!feof(this->fh)) {
			//Check for content signature
			count = read(&magic, 4);
			if (count == 4 && memcmp(&magic, CONTAINER_MAGIC, 4) == 0) {
				read(&key, 4);
				key = READ_LE_UINT32(&key);
				this->CodeTable = create_dec_table(key);
				this->cabinet_offset = ftell(this->fh);
				
				//Check for cabinet signature
				count = read(&magic, 4);
				if (count == 4 && memcmp(&magic, CABINET_MAGIC, 4) == 0) {
					break;
				} else {
					delete [] this->CodeTable;
					this->CodeTable = NULL;
					continue;
				}
			}
		}
		
		seek((off_t) 0, SEEKMODE_START);
	}
	inited = true;
}

void PackFile::close() {
	if (!inited) {
		return;
	}
	if (CodeTable)
		delete CodeTable;
	fclose(fh);
}

int PackFile::seek(off_t offset, int mode) {
	switch (mode) {
		case SEEKMODE_START:
			mode = SEEK_SET;
			if (CodeTable)
				offset += cabinet_offset;
			break;
		case SEEKMODE_CUR:   mode = SEEKMODE_CUR; break;
		case SEEKMODE_END:   mode = SEEKMODE_END; break;
		default: return -1;
	}
	return fseek(fh, (int)offset, mode);
}

int PackFile::read(void *buffer, int bytes) {
	unsigned int start_point = (unsigned int)tell();
	size_t count = fread(buffer, 1, (size_t) bytes, fh);
	
	if (!ferror(this->fh)) {
		if (CodeTable)
			decode((uint8*)buffer, count, CodeTable, start_point);
		return (int) count;
	}
	return -1;
}

#define MSPACK_ERR_OK          (0)
#define MSPACK_ERR_SEEK        (5)
#define MSPACK_ERR_DATAFORMAT  (8)


std::string PackFile::read_string(int *error)
{
	off_t base = this->tell();
	char buf[256];
	unsigned int len, i, ok;
	
	len = this->read(&buf[0], 256);
	
	for (i = 0, ok = 0; i < len; i++) if (!buf[i]) { ok = 1; break; }
	if (!ok) {
		*error = MSPACK_ERR_DATAFORMAT;
		return NULL;
	}
	
	len = i + 1;
	
	if (this->seek(base + (off_t)len, PackFile::SEEKMODE_START)) {
		*error = MSPACK_ERR_SEEK;
		return NULL;
	}
	
	std::string str(buf);
	//	memcpy(str, &buf[0], len);
	*error = MSPACK_ERR_OK;
	return str;
}

int PackFile::write(void *buffer, int bytes) {
	if (CodeTable)
		return -1;
	size_t count = fwrite(buffer, 1, (size_t)bytes, fh);
	if (!ferror(fh)) return (int) count;
	
	return -1;
}

off_t PackFile::tell() {
	off_t offset = ftell(this->fh);
	offset -= this->cabinet_offset;
	return offset;
}