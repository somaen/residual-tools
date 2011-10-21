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
 * $URL: https://residual.svn.sourceforge.net/svnroot/residual/residual/trunk/tools/patchex/mspack.h $
 * $Id: mspack.h 1359 2009-05-26 14:04:08Z aquadran $
 *
 */

/* libmspack -- a library for working with Microsoft compression formats.
 * (C) 2003-2004 Stuart Caie <kyzer@4u.net>
 *
 * This source code is adopted and striped for Residual project.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef LIB_MSPACK_H
#define LIB_MSPACK_H

#include <sys/types.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

struct mspack_file;

class mspack_system {
public:
	virtual mspack_file *open(std::string filename, int mode) = 0;
	virtual void close(mspack_file *file) = 0;
	virtual int read(mspack_file *file, void *buffer, int bytes) = 0;
	virtual int write(mspack_file *file, void *buffer, int bytes) = 0;
	virtual int seek(mspack_file *file, off_t offset, int mode) = 0;
	virtual off_t tell(mspack_file *file) = 0;
	void *null_ptr;
};

class res_system : public mspack_system {
public:
	mspack_file *open(std::string filename, int mode);
	void close(struct mspack_file *file);
	int read(struct mspack_file *file, void *buffer, int bytes);
	int write(struct mspack_file *file, void *buffer, int bytes);
	int seek(struct mspack_file *file, off_t offset, int mode);
	off_t tell(struct mspack_file *file);
	void *null_ptr;
};

#define MSPACK_SYS_OPEN_READ   (0)
#define MSPACK_SYS_OPEN_WRITE  (1)
#define MSPACK_SYS_OPEN_UPDATE (2)
#define MSPACK_SYS_OPEN_APPEND (3)

#define MSPACK_SYS_SEEK_START  (0)
#define MSPACK_SYS_SEEK_CUR    (1)
#define MSPACK_SYS_SEEK_END    (2)

struct mspack_file;

#define MSPACK_ERR_OK          (0)
#define MSPACK_ERR_ARGS        (1)
#define MSPACK_ERR_OPEN        (2)
#define MSPACK_ERR_READ        (3)
#define MSPACK_ERR_WRITE       (4)
#define MSPACK_ERR_SEEK        (5)
#define MSPACK_ERR_NOMEMORY    (6)
#define MSPACK_ERR_SIGNATURE   (7)
#define MSPACK_ERR_DATAFORMAT  (8)
#define MSPACK_ERR_CHECKSUM    (9)
#define MSPACK_ERR_CRUNCH      (10)
#define MSPACK_ERR_DECRUNCH    (11)

extern struct mscab_decompressor *mspack_create_cab_decompressor();

struct mscabd_file {
	mscabd_file *next;
	std::string filename;
	unsigned int length;
	int attribs;
	char time_h;
	char time_m;
	char time_s;
	char date_d;
	char date_m;
	int date_y;
	struct mscabd_folder *folder;
	unsigned int offset;
};

class CabFile {
	mscab_decompressor *_cabd;
	mscabd_file *_files;
	std::string _filename;
	unsigned int _lang;
	void OpenCAB(std::string filename);
public:
	CabFile(std::string filename);
	~CabFile();
	void Close();
	void SetLanguage(unsigned int);
	void ExtractCabinet();
	void ExtractFiles();
};

#endif
