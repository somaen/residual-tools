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
#include "common/scummsys.h"
#include "tools/patchex/cab.h"
#include "tools/patchex/packfile.h"

// Some useful type and function
//typedef unsigned char byte;

static int cabd_sys_read_block(struct mspack_system *sys, struct mscabd_decompress_state *d, int *out, int ignore_cksum);
static unsigned int cabd_checksum(
								  unsigned char *data, unsigned int bytes, unsigned int cksum);
//std::string read_string(PackFile* fh, int *error)
#define __egi32(a,n) (  ((((unsigned char *) a)[n+3]) << 24) | \
((((unsigned char *) a)[n+2]) << 16) | \
((((unsigned char *) a)[n+1]) <<  8) | \
((((unsigned char *) a)[n+0])))

#define EndGetI32(a) __egi32(a,0)
#define EndGetI16(a) ((((a)[1])<<8)|((a)[0]))

mscab_decompressor::mscab_decompressor() {
	//system	= new res_system();
	d		= NULL;
	error	= MSPACK_ERR_OK;
	_cab	= NULL;
	/*
	param[MSCABD_PARAM_SEARCHBUF] = 131072;
	param[MSCABD_PARAM_FIXMSZIP]  = 0;
	param[MSCABD_PARAM_DECOMPBUF] = 65536;	*/
}

mscab_decompressor::~mscab_decompressor() {
	free_decomp();
	
	if (d) {
		//if (d->infh) {
		//delete d;
		//}//system->close(d->infh);
		 delete d;
	}
}

void mscab_decompressor::free_decomp() {
	if (d && d->_folder && d->_state) {
		switch (d->_comp_type & cffoldCOMPTYPE_MASK) {
			case cffoldCOMPTYPE_MSZIP:   delete d->_state;  break;
		}
		d->_state      = NULL;
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
				}//system->close(d->infh);
				free_decomp();
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
	//	const int CFFOLD_NumBlocks = 0x04;
	//	const int CFFOLD_CompType = 0x06;
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
	printf("Num_files in cabinet: %d\n",num_files);
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
/*	if (x) { 
		delete file;
		return x;
	}*/
	
/*	if (!linkfile) this->_files = file;
	else linkfile->next = file;
	linkfile = file;*/

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
	//fol->data.cab        = (struct mscabd_cabinet *) this;
	//fol->data.offset     = offset + (off_t)	( (unsigned int) EndGetI32(&buf[cffold_DataOffset]) );
	_merge_prev      = NULL;
	_merge_next      = NULL;
}

int mscab_decompressor::extract(mscabd_file *file, std::string filename)
{
	struct mscabd_folder *fol;
	PackFile *fh;
	
	int test = 0;
	if (file) test = 1;
	printf("Extract: %d, %s\n", test, filename.c_str());
	
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
			}//system->close(d->infh);
			d->_incab = fol->_data.cab;
			printf("Data.cab: %s\n",fol->_data.cab->GetFilename().c_str());
			d->_infh = new PackFile(fol->_data.cab->GetFilename(), PackFile::OPEN_READ);
			if (!d->_infh) {
				printf("Crashed here\n");
				return error = MSPACK_ERR_OPEN;	
			}
		}
			printf("Got here\n");
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
		if ((bytes = file->_offset - d->_offset)) {
			internal_error = d->decompress(bytes);
			if (error != MSPACK_ERR_READ) error = internal_error;
		}
		
		if (!error) {
			d->_outfh = fh;
			internal_error = d->decompress((off_t) file->_length);
			if (internal_error != MSPACK_ERR_READ) error = internal_error;
		}
	}
	
	//	system->close(fh);
	delete fh;
	d->_outfh = NULL;
	
	return error;
}

void mscab_decompressor::extract_files() {
	unsigned int files_extracted = 0;
	struct mscabd_file *file;
	std::string filename;
	
	for (file = _cab->_files; file; file = file->_next) {
		if ((filename = file_filter(file)) != "") {
			printf("Filtered file: %s\n",filename.c_str());
			if (extract(file, filename.c_str()) != MSPACK_ERR_OK) {
				printf("Extract error on %s!\n", file->_filename.c_str());
				continue;
			}
			printf("%s extracted as %s\n", file->_filename.c_str(), filename.c_str());
			++files_extracted;
			//delete filename;
		}
	}
	
	printf("%d file(s) extracted.\n", files_extracted);
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
	
	free_decomp();
	
	return error = d->init(fh, ct);
}

mscabd_decompress_state::mscabd_decompress_state() {
	_folder	= NULL;
	_data	= NULL;
	_state	= NULL;
	_infh	= NULL;
	_incab	= NULL;
}


mscabd_decompress_state::~mscabd_decompress_state() {
	delete _state;
	_state = NULL;
}

int mscabd_decompress_state::ZipDecompress(off_t offset) {
	_state->decompress(offset);
/*	if(!outfh)
		return MSPACK_ERR_OK;*/
	char *data = _state->getData();
	unsigned int len = _state->getLen();
	if (_decsys.write(_initfh, data, len) != len)
		return MSPACK_ERR_WRITE;
	else
		return MSPACK_ERR_OK;
}

int mscabd_decompress_state::init(PackFile * fh, unsigned int ct) {
	_comp_type = ct;
	_initfh = fh;
	
	//#define PARAM_FIXMSZIP (0)
	//#define PARAM_DECOMPBUF (65536)
	
	//int internal_error;
	assert ((ct & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
/*	switch (ct & cffoldCOMPTYPE_MASK) {
		case cffoldCOMPTYPE_MSZIP:*/
	_state = new mszipd_stream(&_decsys, fh, fh);
/*			break;
		default:
			return internal_error = MSPACK_ERR_DATAFORMAT;
	}*/
	return MSPACK_ERR_OK;
	//return internal_error = (state) ? MSPACK_ERR_OK : MSPACK_ERR_NOMEMORY;
}

int dec_system::read(PackFile *file, void *buffer, int bytes) {
	mscab_decompressor *handle = (mscab_decompressor *) file;
	unsigned char *buf = (unsigned char *) buffer;
	//mspack_system *sys = handle->getSystem();
	int avail, todo, outlen = 0, ignore_cksum;
	
	const int PARAM_FIXMSZIP = 0;
	
	ignore_cksum = PARAM_FIXMSZIP && ((handle->d->_comp_type & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
	
	todo = bytes;
	while (todo > 0) {
		avail = handle->d->_i_end - handle->d->_i_ptr;
		
		if (avail) {
			if (avail > todo) avail = todo;
			memcpy(buf, handle->d->_i_ptr, (size_t) avail);
			handle->d->_i_ptr += avail;
			buf  += avail;
			todo -= avail;
		}
		else {
			if (handle->d->_block++ >= handle->d->_folder->_num_blocks) {
				handle->error = MSPACK_ERR_DATAFORMAT;
				break;
			}
			
			handle->error = read_block(handle->d, &outlen, ignore_cksum);
			if (handle->error) return -1;
			
			if (handle->d->_block >= handle->d->_folder->_num_blocks) {
			} else {
				if (outlen != CAB_BLOCKMAX) {
					fprintf(stderr, "WARNING; non-maximal data block");
				}
			}
		}
	}
	return bytes - todo;
}

int dec_system::write(struct PackFile *file, void *buffer, int bytes) {
	struct mscab_decompressor *handle = (struct mscab_decompressor *) file;
	handle->d->_offset += bytes;
	if (handle->d->_outfh) {
		return handle->d->_outfh->write(buffer, bytes);
	}
	return bytes;
}

int dec_system::read_block(mscabd_decompress_state *d, int *out, int ignore_cksum) {
	unsigned char hdr[cfdata_SIZEOF];
	unsigned int cksum;
	int len;
	
	d->_i_ptr = d->_i_end = &d->_input[0];
	
	do {
		if (d->_infh->read(&hdr[0], cfdata_SIZEOF) != cfdata_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		
		if (d->_data->cab->_block_resv &&
			d->_infh->seek((off_t) d->_data->cab->_block_resv, PackFile::SEEKMODE_CUR))
		{
			return MSPACK_ERR_SEEK;
		}
		
		len = EndGetI16(&hdr[cfdata_CompressedSize]);
		if (((d->_i_end - d->_i_ptr) + len) > CAB_INPUTMAX) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		if (EndGetI16(&hdr[cfdata_UncompressedSize]) > CAB_BLOCKMAX) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		if (d->_infh->read(d->_i_end, len) != len) {
			return MSPACK_ERR_READ;
		}
		
		if ((cksum = EndGetI32(&hdr[cfdata_CheckSum]))) {
			unsigned int sum2 = cabd_checksum(d->_i_end, (unsigned int) len, 0);
			if (cabd_checksum(&hdr[4], 4, sum2) != cksum) {
				if (!ignore_cksum) return MSPACK_ERR_CHECKSUM;
				fprintf(stderr, "WARNING; bad block checksum found");
			}
		}
		
		d->_i_end += len;
		
		if ((*out = EndGetI16(&hdr[cfdata_UncompressedSize]))) {
			return MSPACK_ERR_OK;
		}
		
		//sys->close(d->infh);
		delete d->_infh;
		d->_infh = NULL;
		
		if (!(d->_data = d->_data->next)) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		d->_incab = d->_data->cab;
		if (!(d->_infh = new PackFile(d->_incab->GetFilename(), PackFile::OPEN_READ)))
		{
			return MSPACK_ERR_OPEN;
		}
		
		if (d->_infh->seek(d->_data->offset, PackFile::SEEKMODE_START)) {
			return MSPACK_ERR_SEEK;
		}
	} while (1);
	
	return MSPACK_ERR_OK;
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

#define LANG_ALL1 "@@"
#define LANG_ALL2 "Common"

//unsigned int lang;

std::string mscab_decompressor::file_filter(const struct mscabd_file *file) {
	const char *kLanguages_ext[] = { "English", "French", "German", "Italian", "Portuguese", "Spanish", NULL};
	const char *kLanguages_code1[] = { "US", "FR", "GE", "IT", "PT", "SP",  NULL };
	const char *kLanguages_code2[] = { "Eng", "Fra", "Deu", "Ita", "Brz", "Esp",  NULL };
	
	char *filename;
	std::string retFilename;
	unsigned int filename_size;
	
	//printf("Lang = %d\n",lang);
	filename_size = file->_filename.length();
	
	//Skip executables and libries
	const char *ext = file->_filename.substr(filename_size - 3).c_str();
	//printf("Filename: %s\n",file->filename.c_str());
	//	printf("File-ext: %s\n",ext);
	if (strcasecmp(ext, "exe") == 0 ||
		strcasecmp(ext, "dll") == 0 ||
		strcasecmp(ext, "flt") == 0 ||
		strcasecmp(ext, "asi") == 0) {
		return "";
	}
	
	filename = new char[filename_size + 1];
	
	//Old-style localization (Grimfandango)
	if (filename_size > 3 &&  file->_filename[2] == '_') {
		//printf("Grim-style\n");
		char file_lang[3];
		sscanf(file->_filename.c_str(), "%2s_%s",file_lang, filename);
		//printf("File_lang: %s\n", file_lang);
		retFilename = std::string(filename);
		if (strcmp(file_lang, kLanguages_code1[lang]) == 0 || strcmp(file_lang, LANG_ALL1) == 0) {
			//	printf("MATCHES LANGUAGE!!!!!!! %s\n", retFilename.c_str());
			delete[] filename;
			return retFilename;
		}
	}
	
	//Folder-style localization (EMI)
	unsigned int lcode_size_com, lcode_size_loc;
	lcode_size_com = strlen(LANG_ALL2);
	lcode_size_loc = strlen(kLanguages_code2[lang]);
	if ((filename_size > lcode_size_com && strncmp(file->_filename.c_str(), LANG_ALL2, lcode_size_com - 1) == 0) ||
	    (filename_size > lcode_size_loc && strncmp(file->_filename.c_str(), kLanguages_code2[lang], lcode_size_loc) == 0) ) {
		char *fn = rindex(file->_filename.c_str(), '\\') + 1;
		if (fn != NULL) {
			retFilename = std::string(fn);
//			strcpy(filename, fn);
			delete[] filename;
			return retFilename;
		}
	}
	
	delete[] filename;
	return "";
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

#define BUFFER_SIZE 		102400
void CabFile::ExtractCabinet() {
	struct PackFile *original_executable, *destination_cabinet;
	char *buffer;
	unsigned int copied_bytes;
	int count;
	
	original_executable = new PackFile(_filename.c_str(), PackFile::OPEN_READ);
	destination_cabinet = new PackFile("original.cab", PackFile::OPEN_WRITE);
	
	buffer = new char[BUFFER_SIZE];
	copied_bytes = 0;
	
	while (copied_bytes < _cabd->getCabLength()) {
		count = original_executable->read(buffer, BUFFER_SIZE);
		destination_cabinet->write(buffer, count);
		copied_bytes  += count;
	}
	printf("Update cabinet extracted as original.cab.\n");
	
	delete[] buffer;
	delete original_executable;
	delete destination_cabinet;
}


void CabFile::ExtractFiles() {
	_cabd->extract_files();
}

void CabFile::OpenCAB(std::string filename)
{/* 
	struct mspack_file *fh;
	
	if (_cab)
		close();
	
	if ((fh = system->open(filename, PackFile::OPEN_READ))) {
		if (_cab = new mscabd_cabinet()) {
			_cab->filename = filename;
			error = read_headers(fh, (mscabd_cabinet_p *) _cab, (off_t) 0, 0);
			if (error) {
				close();
				_cab = NULL;
			}
		}
		else {
			error = MSPACK_ERR_NOMEMORY;
		}
		system->close(fh);
	}
	else {
		error = MSPACK_ERR_OPEN;
	}*/
}
