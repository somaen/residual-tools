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
#include <string>



struct mscab_decompressor;
struct mscabd_file;
struct mscabd_cabinet;
class PackFile;

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
