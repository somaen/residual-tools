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

#ifndef MSPACKFILE_H
#define MSPACKFILE_H


#include <cstdio>

#include "common/scummsys.h"


//namespace Patchex {

class PackFile {
	FILE *fh;
	std::string name;
	uint16 *CodeTable;
	off_t cabinet_offset;
	uint16 *create_dec_table(uint32 key);
	void decode(uint8 *data, unsigned int size, uint16 *dectable, unsigned int start_point);
	bool inited;
public:
	PackFile(std::string filename, int mode);
	~PackFile() { close(); }
	void close();
	int read(void *buffer, int bytes);
	std::string getName() { return name; }
	std::string read_string(int *error);
	int write(void *buffer, int bytes);
	int seek(off_t offset, int mode);
	off_t tell();
	
	enum OpenMode {
		OPEN_READ = 0,
		OPEN_WRITE = 1,
		OPEN_UPDATE = 2,
		OPEN_APPEND = 3
	};
	enum SeekMode {
		SEEKMODE_START = 0,
		SEEKMODE_CUR = 1,
		SEEKMODE_END = 2
	};

};

//}
#endif
