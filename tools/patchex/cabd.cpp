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


#include <cassert>
#include <zlib.h>
#include "common/scummsys.h"
#include "tools/patchex/cab.h"
#include "tools/patchex/packfile.h"

// Some useful type and function
//typedef unsigned char byte;

static unsigned int cabd_checksum(unsigned char *data, unsigned int bytes, unsigned int cksum);

#define __egi32(a,n) (  ((((unsigned char *) a)[n+3]) << 24) | \
((((unsigned char *) a)[n+2]) << 16) | \
((((unsigned char *) a)[n+1]) <<  8) | \
((((unsigned char *) a)[n+0])))

#define EndGetI32(a) __egi32(a,0)
#define EndGetI16(a) ((((a)[1])<<8)|((a)[0]))

mscab_decompressor::mscab_decompressor() {
	d		= NULL;
	error	= MSPACK_ERR_OK;
	_cab	= NULL;
}

mscab_decompressor::~mscab_decompressor() {
	if (d) {
		delete d;
		d = NULL;
	}
}

void mscab_decompressor::open(std::string filename)
{
	struct PackFile *fh;

	if (_cab)
		close();
	
	_cab = new mscabd_cabinet(filename);
	error = _cab->read_headers((off_t) 0, 0);
	if (error) {
		printf("Header-read-error\n");
		close();
		_cab = NULL;
	} 
}

void mscab_decompressor::close()
{
	struct mscabd_folder_data *dat, *ndat;
	struct mscabd_cabinet *cab, *ncab;
	struct mscabd_folder *fol, *nfol;
	struct mscabd_file *fi, *nfi;
	struct mspack_system *sys;
	
	error = MSPACK_ERR_OK;
	
	while (_cab) {
		/* free files */
		for (fi = _cab->_files; fi; fi = nfi) {
			nfi = fi->_next;
			delete fi;
		}
		
		for (fol = _cab->_folders; fol; fol = nfol) {
			nfol = fol->_next;
			
			if (d && (d->_folder == (struct mscabd_folder *) fol)) {
				if (d->_infh) {
					delete d->_infh;
				}
				delete d;
				d = NULL;
			}
			
			for (dat = ((struct mscabd_folder *)fol)->_data.next; dat; dat = ndat) {
				ndat = dat->next;
				delete dat;
			}
			delete fol;
		}
		
		for (cab = _cab; cab; cab = ncab) {
			ncab = cab->_prevcab;
			if (cab != _cab) 
				delete cab;
		}
		
		for (cab = _cab->_nextcab; cab; cab = ncab) {
			ncab = cab->_nextcab;
			delete cab;
		}
		
		cab = _cab->_next;
		delete _cab;
		
		_cab = (mscabd_cabinet *) cab;
	}
	_cab = NULL;
}

mscabd_cabinet::mscabd_cabinet(std::string fname) : _filename(fname) {
	_fh = new PackFile(_filename, PackFile::OPEN_READ);
}

int mscabd_cabinet::read_headers(off_t offset, int quiet)
{
	const int CFHEAD_Signature = 0x00;
	const int CFHEAD_CabinetSize = 0x08;
	const int CFHEAD_FileOffset = 0x10;
	const int CFHEAD_MinorVersion = 0x18;
	const int CFHEAD_MajorVersion = 0x19;
	const int CFHEAD_NumFolders = 0x1A;
	const int CFHEAD_NumFiles = 0x1C;
	const int CFHEAD_Flags = 0x1E;
	const int CFHEAD_SetID = 0x20;
	const int CFHEAD_CabinetIndex = 0x22;
	const int CFHEAD_SIZEOF = 0x24;
	
	// Duplicated in mscab_folder-constructor:
	const int CFFOLD_DataOffset = 0x00;
	const int CFFOLD_SIZEOF = 0x08;
	
	int num_folders, num_files, folder_resv, i, x;
	struct mscabd_folder *fol, *linkfol = NULL;
	struct mscabd_file *file, *linkfile = NULL;
	unsigned char buf[64];
	
	// TODO: Cleanup this's
	this->_next     = NULL;
	this->_files    = NULL;
	this->_folders  = NULL;
	this->_prevcab  = this->_nextcab  = NULL;
	this->_prevname = this->_nextname = "";
	this->_previnfo = this->_nextinfo = "";
	
	this->_base_offset = offset;
	
	if (_fh->seek(offset, PackFile::SEEKMODE_START)) {	
		return MSPACK_ERR_SEEK;
	}
	
	// Read header into buf. 
	if (_fh->read(&buf[0], CFHEAD_SIZEOF) != CFHEAD_SIZEOF) {
		return MSPACK_ERR_READ;
	}
	
	// Verify Head-signature
	if (EndGetI32(&buf[CFHEAD_Signature]) != 0x4643534D) {
		return MSPACK_ERR_SIGNATURE;
	}
	
	// Fill in cabinet-size
	this->_length    = EndGetI32(&buf[CFHEAD_CabinetSize]);
	this->_set_id    = EndGetI16(&buf[CFHEAD_SetID]);
	this->_set_index = EndGetI16(&buf[CFHEAD_CabinetIndex]);
	
	// Get num folders (should be >0)
	num_folders = EndGetI16(&buf[CFHEAD_NumFolders]);
	if (num_folders == 0) {
		if (!quiet) fprintf(stderr, "no folders in cabinet.");
		return MSPACK_ERR_DATAFORMAT;
	}
	
	// Get num files (should be >0)
	num_files = EndGetI16(&buf[CFHEAD_NumFiles]);
	if (num_files == 0) {
		if (!quiet) fprintf(stderr, "no files in cabinet.");
		return MSPACK_ERR_DATAFORMAT;
	}
	
	// Verify version is 1.3
	if ((buf[CFHEAD_MajorVersion] != 1) && (buf[CFHEAD_MinorVersion] != 3)) {
		if (!quiet) fprintf(stderr, "WARNING; cabinet version is not 1.3");
	}
	
	this->_flags = EndGetI16(&buf[CFHEAD_Flags]);
	if (this->_flags & cfheadRESERVE_PRESENT) {
		if (_fh->read(&buf[0], cfheadext_SIZEOF) != cfheadext_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		this->_header_resv = EndGetI16(&buf[cfheadext_HeaderReserved]);
		folder_resv           = buf[cfheadext_FolderReserved];
		this->_block_resv       = buf[cfheadext_DataReserved];
		
		if (this->_header_resv > 60000) {
			if (!quiet) fprintf(stderr, "WARNING; reserved header > 60000.");
		}
		
		if (this->_header_resv) {
			if (_fh->seek((off_t) this->_header_resv, PackFile::SEEKMODE_CUR)) {
				return MSPACK_ERR_SEEK;
			}
		}
	}
	else {
		this->_header_resv = 0;
		folder_resv        = 0; 
		this->_block_resv  = 0;
	}
	
	// Fill in linked list-pointers
	if (this->_flags & cfheadPREV_CABINET) {
		this->_prevname = _fh->read_string(&x); if (x) return x;
		this->_previnfo = _fh->read_string(&x); if (x) return x;
	}
	
	if (this->_flags & cfheadNEXT_CABINET) {
		this->_nextname = _fh->read_string(&x); if (x) return x;
		this->_nextinfo = _fh->read_string(&x); if (x) return x;
	}
	// Read in folders
	for (i = 0; i < num_folders; i++) {
		if (_fh->read(&buf[0], CFFOLD_SIZEOF) != CFFOLD_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		if (folder_resv) {
			if (_fh->seek((off_t) folder_resv, PackFile::SEEKMODE_CUR)) {
				return MSPACK_ERR_SEEK;
			}
		}
		
		if (!(fol = new mscabd_folder(buf))) {
			return MSPACK_ERR_NOMEMORY;
		}

		fol->_data.cab        = (struct mscabd_cabinet *) this;
		fol->_data.offset     = offset + (off_t) ( (unsigned int) EndGetI32(&buf[CFFOLD_DataOffset]) );
		
		if (!linkfol) this->_folders = (struct mscabd_folder *) fol;
		else linkfol->_next = (struct mscabd_folder *) fol;
		linkfol = fol;
	}
	
	for (i = 0; i < num_files; i++) {
		if (!(file = new mscabd_file(_fh, this))) {
			return MSPACK_ERR_NOMEMORY;
		}
		
		if (!linkfile) this->_files = file;
		else linkfile->_next = file;
		linkfile = file;
	}
	
	return MSPACK_ERR_OK;
}

// Requires the packfile to be seeked to the start of a cabd_file, obviously.
mscabd_file::mscabd_file(PackFile * fh, mscabd_cabinet *cab) {
	const int cffile_UncompressedSize = (0x00);
	const int cffile_FolderOffset     = (0x04);
	const int cffile_FolderIndex      = (0x08);
	const int cffile_Date             = (0x0A);
	const int cffile_Time             = (0x0C);
	const int cffile_Attribs          = (0x0E);
	const int cffile_SIZEOF           = (0x10);
	
	const int CFFILE_Continued_from_prev = 0xFFFD;
	const int CFFILE_Continued_to_next = 0xFFFE;
	const int CFFILE_Continued_prev_and_next = 0xFFFF;
	
	unsigned char buf[64];
	int x;
	struct mscabd_folder *fol;
	if (fh->read(&buf[0], cffile_SIZEOF) != cffile_SIZEOF) {
		assert(0);
		//return MSPACK_ERR_READ;
	}
	
	_next     = NULL;
	_length   = EndGetI32(&buf[cffile_UncompressedSize]);
	_attribs  = EndGetI16(&buf[cffile_Attribs]);
	_offset   = EndGetI32(&buf[cffile_FolderOffset]);
	
	x = EndGetI16(&buf[cffile_FolderIndex]);
	if (x < CFFILE_Continued_from_prev) {
		struct mscabd_folder *ifol = cab->_folders; 
		while (x--) if (ifol) ifol = ifol->_next;
		_folder = ifol;
		
		assert(ifol);
		if (!ifol) {
			//delete file;
			//return MSPACK_ERR_DATAFORMAT;
		}
	}
	else {
		if ((x == CFFILE_Continued_to_next) ||
			(x == CFFILE_Continued_prev_and_next))
		{
			struct mscabd_folder *ifol = cab->_folders;
			while (ifol->_next) ifol = ifol->_next;
			_folder = ifol;
			
			fol = (struct mscabd_folder *) ifol;
			if (!fol->_merge_next) fol->_merge_next = this;
		}
		
		if ((x == CFFILE_Continued_from_prev) ||
			(x == CFFILE_Continued_prev_and_next))
		{
			_folder = cab->_folders;
			
			fol = (struct mscabd_folder *) _folder;
			if (!fol->_merge_prev) fol->_merge_prev = this;
		}
	}
	x = EndGetI16(&buf[cffile_Time]);
	_time_h = x >> 11;
	_time_m = (x >> 5) & 0x3F;
	_time_s = (x << 1) & 0x3E;
	
	x = EndGetI16(&buf[cffile_Date]);
	_date_d = x & 0x1F;
	_date_m = (x >> 5) & 0xF;
	_date_y = (x >> 9) + 1980;
	
	_filename = fh->read_string(&x);
	assert(!x);

}


mscabd_folder::mscabd_folder(unsigned char buf[64]) {
	const int CFFOLD_DataOffset = 0x00;
	const int CFFOLD_NumBlocks = 0x04;
	const int CFFOLD_CompType = 0x06;
	const int CFFOLD_SIZEOF = 0x08;
	_next       = NULL;
	_comp_type  = EndGetI16(&buf[CFFOLD_CompType]);
	_num_blocks = EndGetI16(&buf[CFFOLD_NumBlocks]);
	_data.next       = NULL;
	_merge_prev      = NULL;
	_merge_next      = NULL;
}

int mscab_decompressor::extract(mscabd_file *file, std::string filename)
{
	struct mscabd_folder *fol;
	PackFile *fh;
	
	if (!file) return error = MSPACK_ERR_ARGS;
	
	// Get the folder reference from the file.
	fol = file->_folder;
	
	/* check if file can be extracted */
	if ((!fol) || (fol->_merge_prev) ||
		(((file->_offset + file->_length) / CAB_BLOCKMAX) > fol->_num_blocks))
	{
		fprintf(stderr, "ERROR; file \"%s\" cannot be extracted, "
				"cabinet set is incomplete.", file->_filename.c_str());
		return error = MSPACK_ERR_DATAFORMAT;
	}
	
	if (d) {
		delete d;
		d = NULL;
	}
	
	if (!d) {
		d = new mscabd_decompress_state();
		if (!d) return error = MSPACK_ERR_NOMEMORY;
	}
	
	// Does our current state have the right folder selected?
	if ((d->_folder != fol) || (d->_offset > file->_offset)) {
		if (!d->_infh || (fol->_data.cab != d->_incab)) {
			if (d->_infh) {
				delete d->_infh;
				d->_infh = NULL;
			}
			d->_incab = fol->_data.cab;
			d->_infh = new PackFile(fol->_data.cab->GetFilename(), PackFile::OPEN_READ);
			if (!d->_infh) {
				return error = MSPACK_ERR_OPEN;	
			}
		}

		if (d->_infh->seek(fol->_data.offset, PackFile::SEEKMODE_START)) {
			return error = MSPACK_ERR_SEEK;
		}
		
		if (init_decomp((unsigned int) fol->_comp_type)) {
			return error;
		}
		
		d->_folder = fol;
		d->_data   = &fol->_data;
		d->_offset = 0;
		d->_block  = 0;
		d->_i_ptr = d->_i_end = &d->_input[0];
	}
	
	if (!(fh = new PackFile(filename, PackFile::OPEN_WRITE))) {
		return error = MSPACK_ERR_OPEN;
	}

	
	error = MSPACK_ERR_OK;

	if (file->_length) {
		off_t bytes;
		int internal_error;
		d->_outfh = NULL;
		
		if (!error) {
			d->_outfh = fh;
			internal_error = d->decompress(d->_offset, file->_offset, file->_length);
		}
	}
	
	delete fh;
	d->_outfh = NULL;
	
	return error;
}

int mscab_decompressor::last_error() {
	return error;
}

void mscab_decompressor::printFiles() {
	char *filename;
	struct mscabd_file *file;
	
	for (file = _cab->_files; file; file = file->_next) {
		printf("Cabinet contains %s!\n", file->_filename.c_str());
	}	
}

int mscab_decompressor::init_decomp(unsigned int ct)
{
	struct PackFile *fh = (struct PackFile *) this;
	
	return error = d->init(fh, ct);
}

mscabd_decompress_state::mscabd_decompress_state() {
	_folder		= NULL;
	_data		= NULL;
	_infh		= NULL;
	_incab		= NULL;
	_fileBuf	= NULL;
	_fileBufLen	= 0;
}

mscabd_decompress_state::~mscabd_decompress_state() {
	if (_fileBuf) {
		delete[] _fileBuf;
	}
}
int mscabd_decompress_state::ZipDecompress(off_t preread, off_t offset, off_t length) {
	if (_fileBuf) {
		delete[] _fileBuf;
		_fileBuf = NULL;
	}
	// len = the entire block decompressed.
	// length = the size of the file at hand.
	_fileBuf = new char[length];
	Bytef *data;
	unsigned int len = zlibDecompress(preread, offset, length, data);
	memcpy(_fileBuf, (data+offset), length);
	_fileBufLen = length;
	delete[] data; // Delete the buffer containing all the data, as we have copied out what we need, this is not the best way though.
	return MSPACK_ERR_OK;
}

int mscabd_decompress_state::zlibDecompress(off_t preread, off_t offset, off_t length, Bytef *&ret) {
	// Ref: http://blogs.kde.org/node/3181
	Bytef *in;
	int size = offset - preread + length;
	int outsize = 0;
	int decompressed = 0;
	const uint32 block = 32768;
	in = new Bytef[block+12]; //According to MSZIP specification this is the maximum size of a block
	Bytef *in_tmp = in;
	Bytef *dest = new Bytef[size + block]; // 8 MiB ought to be enough.
	Bytef *dest_tmp = dest;
	ret = new Bytef[size * 16]; // FIXME: This buffer shouldn't need to be this big.
	Bytef * ret_tmp = ret;
	
	int success = 0;
	z_stream_s zStream;
	
	zStream.next_in = Z_NULL;
	zStream.avail_in = 0;
	zStream.zalloc = Z_NULL;
	zStream.zfree = Z_NULL;
	zStream.opaque = Z_NULL;
	
	success = inflateInit2(&zStream, -MAX_WBITS);

	if(success != Z_OK){
		printf("ZLIB failed to initialize\n");
		return 0;
	}
	// Headerparse
	
	unsigned char hdr[cfdata_SIZEOF];
	unsigned int cksum;
	int len;
	
	const int PARAM_FIXMSZIP = 0;
	int ignore_cksum = PARAM_FIXMSZIP && ((_comp_type & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
	ignore_cksum = 1;
	while (decompressed < size) {
		in = in_tmp;

		// Read header
		if (_infh->read(&hdr[0], cfdata_SIZEOF) != cfdata_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		
		if (_data->cab->_block_resv && _infh->seek((off_t) _data->cab->_block_resv, PackFile::SEEKMODE_CUR))
		{
			return MSPACK_ERR_SEEK;
		}

		len = EndGetI16(&hdr[cfdata_CompressedSize]);

		int uncompressed = EndGetI16(&hdr[cfdata_UncompressedSize]);

		if (EndGetI16(&hdr[cfdata_UncompressedSize]) > CAB_BLOCKMAX) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		if (_infh->read(in, len) != len) {
			return MSPACK_ERR_READ;
		}
			
		if ((cksum = EndGetI32(&hdr[cfdata_CheckSum]))) {
			unsigned int sum2 = cabd_checksum(in, (unsigned int) len, 0);
			if (cabd_checksum(&hdr[4], 4, sum2) != cksum) {
				if (!ignore_cksum) return MSPACK_ERR_CHECKSUM;
				fprintf(stderr, "WARNING; bad block checksum found\n");
			}
		}
		
		in+=2; // Skip the CK-header

		int avail, todo, outlen = 0, ignore_cksum;
		avail = len;

		zStream.avail_in = avail;
		zStream.next_in = in;
		zStream.avail_out = block;
		zStream.next_out = dest;
	
		success = inflate(&zStream, Z_SYNC_FLUSH);
		
		decompressed += zStream.total_out;

		memcpy(ret_tmp, dest, zStream.total_out);
		ret_tmp+=zStream.total_out;
		
		outsize = decompressed;
		outsize = zStream.total_out;
		
		if(success != Z_STREAM_END) {
			printf("ERROR: decompressed size bigger than 64 KiB\n");
			return 0;
		}
	
		inflateReset(&zStream);
		inflateSetDictionary(&zStream, dest, outsize);
	}
	
	inflateEnd(&zStream);
	delete[] dest;
	return size;
}

int mscabd_decompress_state::init(PackFile * fh, unsigned int ct) {
	_comp_type = ct;
	_initfh = fh;
	
	assert ((ct & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
	return MSPACK_ERR_OK;
}

int mscabd_decompress_state::write(struct PackFile *file, void *buffer, int bytes) {
	struct mscab_decompressor *handle = (struct mscab_decompressor *) file;
	if (_outfh) {
		return _outfh->write(buffer, bytes);
	}
	return bytes;
}

static unsigned int cabd_checksum(unsigned char *data, unsigned int bytes, unsigned int cksum) {
	unsigned int len, ul = 0;
	
	for (len = bytes >> 2; len--; data += 4) {
		cksum ^= ((data[0]) | (data[1]<<8) | (data[2]<<16) | (data[3]<<24));
	}
	
	switch (bytes & 3) {
		case 3: ul |= *data++ << 16;
		case 2: ul |= *data++ <<  8;
		case 1: ul |= *data;
	}
	cksum ^= ul;
	
	return cksum;
}

// CabFile

CabFile::CabFile(std::string filename) {
	_filename = filename;
	_cabd = new mscab_decompressor();
	_cabd->open(_filename);
	assert(!_cabd->last_error());
}
CabFile::~CabFile() {
	delete _cabd;
}
void CabFile::Close() {
	delete _cabd;
	_cabd = NULL;
}
void CabFile::SetLanguage(unsigned int lang) {
	_lang = lang;
	_cabd->lang = _lang;
}

void CabFile::ExtractCabinet() {
	const unsigned int BUFFER_SIZE = 102400;
	struct PackFile *original_executable, *destination_cabinet;
	char *buffer;
	unsigned int copied_bytes, remBytes, lenght;
	int count, writeResult;
	
	original_executable = new PackFile(_filename.c_str(), PackFile::OPEN_READ);
	destination_cabinet = new PackFile("original.cab", PackFile::OPEN_WRITE);
	
	buffer = new char[BUFFER_SIZE];
	copied_bytes = 0;
	
	lenght =  _cabd->getCabLength();
	while (copied_bytes < lenght) {
		remBytes = lenght - copied_bytes;
		count = original_executable->read(buffer, (remBytes < BUFFER_SIZE) ? remBytes : BUFFER_SIZE);
		writeResult = destination_cabinet->write(buffer, count);
		copied_bytes  += count;

		if (count < 0 || writeResult < 0) {
			printf("I/O Error!\n");
			delete[] buffer;
			delete original_executable;
			delete destination_cabinet;
			exit(1);
		}
	}
	printf("Update cabinet extracted as original.cab.\n");
	
	delete[] buffer;
	delete original_executable;
	delete destination_cabinet;
}

void CabFile::ExtractFiles() {
	unsigned int files_extracted = 0;
	struct mscabd_file *file;
	std::string filename;
	
	for (file = _cabd->getCab()->_files; file; file = file->_next) {
		if ((filename = FileFilter(file)) != "") {
			if (_cabd->extract(file, filename.c_str()) != MSPACK_ERR_OK) {
				printf("Extract error on %s!\n", file->_filename.c_str());
				continue;
			}
			Write(filename, _cabd->d->getFileBuf(), _cabd->d->getFileBufLen());
			printf("%s extracted as %s\n", file->_filename.c_str(), filename.c_str());
			++files_extracted;
		}
	}
	
	printf("%d file(s) extracted.\n", files_extracted);
}

void CabFile::Extract(std::string filename) {
	struct mscabd_file *file;
	std::string fname;
	for (file = _cabd->getCab()->_files; file; file = file->_next) {
		if ((fname = FileFilter(file)) == filename || filename == file->_filename) {
			if (_cabd->extract(file, fname.c_str()) != MSPACK_ERR_OK) {
				printf("Extract error on %s!\n", file->_filename.c_str());
			}
			Write(fname, _cabd->d->getFileBuf(), _cabd->d->getFileBufLen());
			printf("%s extracted as %s\n", file->_filename.c_str(), filename.c_str());
			break;
		}
	}
}

void CabFile::Write(std::string filename, char *data, unsigned int length) {
	PackFile *fh = new PackFile(filename, PackFile::OPEN_WRITE);
	fh->write(data, length);
	delete fh;
}

#define LANG_ALL "@@"
std::string CabFile::FileFilter(const struct mscabd_file *file) {
	const char *kLanguages_ext[] = { "English", "French", "German", "Italian", "Portuguese", "Spanish", NULL};
	const char *kLanguages_code[] = { "US", "FR", "GE", "IT", "PT", "SP",  NULL };
	
	char *filename = NULL;
	std::string retFilename;
	unsigned int filename_size;
	
	filename_size = file->_filename.length();
	
	/* Skip executables and libraries
	 * These files are useless for Residual and a proper extraction of these
	 * requires sub-folder support, so it isn't implemented. */
	const char *ext = file->_filename.substr(filename_size - 3).c_str();
	
	if (strcasecmp(ext, "exe") == 0 ||
		strcasecmp(ext, "dll") == 0 ||
		strcasecmp(ext, "flt") == 0 ||
		strcasecmp(ext, "asi") == 0) {
		return "";
	}
	
	filename = new char[filename_size + 1];
	
	//Old-style localization (Grimfandango)
	if (filename_size > 3 &&  file->_filename[2] == '_') {
		char file_lang[3];
		sscanf(file->_filename.c_str(), "%2s_%s",file_lang, filename);
		retFilename = std::string(filename);
		if (strcmp(file_lang, kLanguages_code[_lang]) == 0 || strcmp(file_lang, LANG_ALL) == 0) {
			delete[] filename;
			return retFilename;
		}
	}
	
	/* Folder-style localization (EMI) Because EMI updates aren't multi-language,
	 * every file is extracted (except for Win's binaries). Subfolders are ignored */
	strcpy(filename, file->_filename.c_str());
	char *fn = strchr(filename, '\\');
	if (fn != NULL && fn[0] != 0) {
		retFilename = std::string(fn);
		delete[] filename;
		return retFilename;
	}
	
	delete[] filename;
	return std::string("");
}
