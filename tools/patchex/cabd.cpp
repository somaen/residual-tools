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

mscabd_cabinet::mscabd_cabinet(std::string fname) : filename(fname) {
	_fh = new PackFile(filename, PackFile::OPEN_READ);
}

static int cabd_sys_read_block(struct mspack_system *sys, struct mscabd_decompress_state *d, int *out, int ignore_cksum);
static unsigned int cabd_checksum(
								  unsigned char *data, unsigned int bytes, unsigned int cksum);

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
	if (d && d->folder && d->state) {
		switch (d->comp_type & cffoldCOMPTYPE_MASK) {
			case cffoldCOMPTYPE_MSZIP:   delete d->state;  break;
		}
		d->state      = NULL;
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
		for (fi = _cab->files; fi; fi = nfi) {
			nfi = fi->next;
			delete fi;
		}
		
		for (fol = _cab->folders; fol; fol = nfol) {
			nfol = fol->next;
			
			if (d && (d->folder == (struct mscabd_folder *) fol)) {
				if (d->infh) {
					delete d->infh;
				}//system->close(d->infh);
				free_decomp();
				delete d;
				d = NULL;
			}
			
			for (dat = ((struct mscabd_folder *)fol)->data.next; dat; dat = ndat) {
				ndat = dat->next;
				delete dat;
			}
			delete fol;
		}
		
		for (cab = _cab; cab; cab = ncab) {
			ncab = cab->prevcab;
			if (cab != _cab) 
				delete cab;
		}
		
		for (cab = _cab->nextcab; cab; cab = ncab) {
			ncab = cab->nextcab;
			delete cab;
		}
		
		cab = _cab->next;
		delete _cab;
		
		_cab = (mscabd_cabinet *) cab;
	}
	_cab = NULL;
}

int mscabd_cabinet::read_headers(off_t offset, int quiet)
{
	int num_folders, num_files, folder_resv, i, x;
	struct mscabd_folder *fol, *linkfol = NULL;
	struct mscabd_file *file, *linkfile = NULL;
	unsigned char buf[64];
	
	this->next     = NULL;
	this->files    = NULL;
	this->folders  = NULL;
	this->prevcab  = this->nextcab  = NULL;
	this->prevname = this->nextname = "";
	this->previnfo = this->nextinfo = "";
	
	this->base_offset = offset;
	
	if (_fh->seek(offset, PackFile::SEEKMODE_START)) {	
		return MSPACK_ERR_SEEK;
	}
	
	if (_fh->read(&buf[0], cfhead_SIZEOF) != cfhead_SIZEOF) {
		return MSPACK_ERR_READ;
	}
	
	if (EndGetI32(&buf[cfhead_Signature]) != 0x4643534D) {
		return MSPACK_ERR_SIGNATURE;
	}
	
	this->length    = EndGetI32(&buf[cfhead_CabinetSize]);
	this->set_id    = EndGetI16(&buf[cfhead_SetID]);
	this->set_index = EndGetI16(&buf[cfhead_CabinetIndex]);
	
	num_folders = EndGetI16(&buf[cfhead_NumFolders]);
	if (num_folders == 0) {
		if (!quiet) fprintf(stderr, "no folders in cabinet.");
		return MSPACK_ERR_DATAFORMAT;
	}
	
	num_files = EndGetI16(&buf[cfhead_NumFiles]);
	if (num_files == 0) {
		if (!quiet) fprintf(stderr, "no files in cabinet.");
		return MSPACK_ERR_DATAFORMAT;
	}
	
	if ((buf[cfhead_MajorVersion] != 1) && (buf[cfhead_MinorVersion] != 3)) {
		if (!quiet) fprintf(stderr, "WARNING; cabinet version is not 1.3");
	}
	
	this->flags = EndGetI16(&buf[cfhead_Flags]);
	if (this->flags & cfheadRESERVE_PRESENT) {
		if (_fh->read(&buf[0], cfheadext_SIZEOF) != cfheadext_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		this->header_resv = EndGetI16(&buf[cfheadext_HeaderReserved]);
		folder_resv           = buf[cfheadext_FolderReserved];
		this->block_resv       = buf[cfheadext_DataReserved];
		
		if (this->header_resv > 60000) {
			if (!quiet) fprintf(stderr, "WARNING; reserved header > 60000.");
		}
		
		if (this->header_resv) {
			if (_fh->seek((off_t) this->header_resv, PackFile::SEEKMODE_CUR)) {
				return MSPACK_ERR_SEEK;
			}
		}
	}
	else {
		this->header_resv = 0;
		folder_resv           = 0; 
		this->block_resv       = 0;
	}
	
	if (this->flags & cfheadPREV_CABINET) {
		this->prevname = read_string(&x); if (x) return x;
		this->previnfo = read_string(&x); if (x) return x;
	}
	
	if (this->flags & cfheadNEXT_CABINET) {
		this->nextname = read_string(&x); if (x) return x;
		this->nextinfo = read_string(&x); if (x) return x;
	}
	
	for (i = 0; i < num_folders; i++) {
		if (_fh->read(&buf[0], cffold_SIZEOF) != cffold_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		if (folder_resv) {
			if (_fh->seek((off_t) folder_resv, PackFile::SEEKMODE_CUR)) {
				return MSPACK_ERR_SEEK;
			}
		}
		
		if (!(fol = new mscabd_folder())) {
			return MSPACK_ERR_NOMEMORY;
		}
		fol->next       = NULL;
		fol->comp_type  = EndGetI16(&buf[cffold_CompType]);
		fol->num_blocks = EndGetI16(&buf[cffold_NumBlocks]);
		fol->data.next       = NULL;
		fol->data.cab        = (struct mscabd_cabinet *) this;
		fol->data.offset     = offset + (off_t)
		( (unsigned int) EndGetI32(&buf[cffold_DataOffset]) );
		fol->merge_prev      = NULL;
		fol->merge_next      = NULL;
		
		if (!linkfol) this->folders = (struct mscabd_folder *) fol;
		else linkfol->next = (struct mscabd_folder *) fol;
		linkfol = fol;
	}
	
	for (i = 0; i < num_files; i++) {
		if (_fh->read(&buf[0], cffile_SIZEOF) != cffile_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		
		if (!(file = new mscabd_file())) {
			return MSPACK_ERR_NOMEMORY;
		}
		
		file->next     = NULL;
		file->length   = EndGetI32(&buf[cffile_UncompressedSize]);
		file->attribs  = EndGetI16(&buf[cffile_Attribs]);
		file->offset   = EndGetI32(&buf[cffile_FolderOffset]);
		
		x = EndGetI16(&buf[cffile_FolderIndex]);
		if (x < cffileCONTINUED_FROM_PREV) {
			struct mscabd_folder *ifol = this->folders; 
			while (x--) if (ifol) ifol = ifol->next;
			file->folder = ifol;
			
			if (!ifol) {
				delete file;
				return MSPACK_ERR_DATAFORMAT;
			}
		}
		else {
			if ((x == cffileCONTINUED_TO_NEXT) ||
				(x == cffileCONTINUED_PREV_AND_NEXT))
			{
				struct mscabd_folder *ifol = this->folders;
				while (ifol->next) ifol = ifol->next;
				file->folder = ifol;
				
				fol = (struct mscabd_folder *) ifol;
				if (!fol->merge_next) fol->merge_next = file;
			}
			
			if ((x == cffileCONTINUED_FROM_PREV) ||
				(x == cffileCONTINUED_PREV_AND_NEXT))
			{
				file->folder = this->folders;
				
				fol = (struct mscabd_folder *) file->folder;
				if (!fol->merge_prev) fol->merge_prev = file;
			}
		}
		
		x = EndGetI16(&buf[cffile_Time]);
		file->time_h = x >> 11;
		file->time_m = (x >> 5) & 0x3F;
		file->time_s = (x << 1) & 0x3E;
		
		x = EndGetI16(&buf[cffile_Date]);
		file->date_d = x & 0x1F;
		file->date_m = (x >> 5) & 0xF;
		file->date_y = (x >> 9) + 1980;
		
		file->filename = read_string(&x);
		if (x) { 
			delete file;
			return x;
		}
		
		if (!linkfile) this->files = file;
		else linkfile->next = file;
		linkfile = file;
	}
	
	return MSPACK_ERR_OK;
}

std::string mscabd_cabinet::read_string(int *error)
{
	off_t base = _fh->tell();
	char buf[256];
	unsigned int len, i, ok;
	
	len = _fh->read(&buf[0], 256);
	
	for (i = 0, ok = 0; i < len; i++) if (!buf[i]) { ok = 1; break; }
	if (!ok) {
		*error = MSPACK_ERR_DATAFORMAT;
		return NULL;
	}
	
	len = i + 1;
	
	if (_fh->seek(base + (off_t)len, PackFile::SEEKMODE_START)) {
		*error = MSPACK_ERR_SEEK;
		return NULL;
	}
	
	std::string str(buf);
	//	memcpy(str, &buf[0], len);
	*error = MSPACK_ERR_OK;
	return str;
}

int mscab_decompressor::extract(mscabd_file *file, std::string filename)
{
	struct mscabd_folder *fol;
	PackFile *fh;
	
	int test = 0;
	if (file) test = 1;
	printf("Extract: %d, %s\n", test, filename.c_str());
	
	if (!file) return error = MSPACK_ERR_ARGS;
	
	fol = file->folder;
	
	/* check if file can be extracted */
	if ((!fol) || (fol->merge_prev) ||
		(((file->offset + file->length) / CAB_BLOCKMAX) > fol->num_blocks))
	{
		fprintf(stderr, "ERROR; file \"%s\" cannot be extracted, "
				"cabinet set is incomplete.", file->filename.c_str());
		return error = MSPACK_ERR_DATAFORMAT;
	}
	
	if (!d) {
		d = new mscabd_decompress_state();
		if (!d) return error = MSPACK_ERR_NOMEMORY;
	}
	
	if ((d->folder != fol) || (d->offset > file->offset)) {
		if (!d->infh || (fol->data.cab != d->incab)) {
			if (d->infh) {
				delete d->infh;
				d->infh = NULL;
			}//system->close(d->infh);
			d->incab = fol->data.cab;
			printf("Data.cab: %s\n",fol->data.cab->GetFilename().c_str());
			d->infh = new PackFile(fol->data.cab->GetFilename(), PackFile::OPEN_READ);
			if (!d->infh) {
				printf("Crashed here\n");
				return error = MSPACK_ERR_OPEN;	
			}
		}
			printf("Got here\n");
		if (d->infh->seek(fol->data.offset, PackFile::SEEKMODE_START)) {
			return error = MSPACK_ERR_SEEK;
		}
		
		if (init_decomp((unsigned int) fol->comp_type)) {
			return error;
		}
		
		d->folder = fol;
		d->data   = &fol->data;
		d->offset = 0;
		d->block  = 0;
		d->i_ptr = d->i_end = &d->input[0];
	}
	
	if (!(fh = new PackFile(filename, PackFile::OPEN_WRITE))) {
		return error = MSPACK_ERR_OPEN;
	}

	
	error = MSPACK_ERR_OK;
	
	if (file->length) {
		off_t bytes;
		int internal_error;
		d->outfh = NULL;
		if ((bytes = file->offset - d->offset)) {
			internal_error = d->decompress(bytes);
			if (error != MSPACK_ERR_READ) error = internal_error;
		}
		
		if (!error) {
			d->outfh = fh;
			internal_error = d->decompress((off_t) file->length);
			if (internal_error != MSPACK_ERR_READ) error = internal_error;
		}
	}
	
	//	system->close(fh);
	delete fh;
	d->outfh = NULL;
	
	return error;
}

void mscab_decompressor::extract_files() {
	unsigned int files_extracted = 0;
	struct mscabd_file *file;
	std::string filename;
	
	for (file = _cab->files; file; file = file->next) {
		if ((filename = file_filter(file)) != "") {
			printf("Filtered file: %s\n",filename.c_str());
			if (extract(file, filename.c_str()) != MSPACK_ERR_OK) {
				printf("Extract error on %s!\n", file->filename.c_str());
				continue;
			}
			printf("%s extracted as %s\n", file->filename.c_str(), filename.c_str());
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
	
	for (file = _cab->files; file; file = file->next) {
		printf("Cabinet contains %s!\n", file->filename.c_str());
	}	
}

int mscab_decompressor::init_decomp(unsigned int ct)
{
	struct PackFile *fh = (struct PackFile *) this;
	
	free_decomp();
	
	return error = d->init(fh, ct);
}

mscabd_decompress_state::mscabd_decompress_state() {
		folder     = NULL;
		data       = NULL;
		state      = NULL;
		infh       = NULL;
		incab      = NULL;
}


mscabd_decompress_state::~mscabd_decompress_state() {
	delete state;
	state = NULL;
}

int mscabd_decompress_state::ZipDecompress(off_t offset) {
	state->decompress(offset);
/*	if(!outfh)
		return MSPACK_ERR_OK;*/
	char *data = state->getData();
	unsigned int len = state->getLen();
	if (decsys.write(initfh, data, len) != len)
		return MSPACK_ERR_WRITE;
	else
		return MSPACK_ERR_OK;
}

int mscabd_decompress_state::init(PackFile * fh, unsigned int ct) {
	comp_type = ct;
	initfh = fh;
	
#define PARAM_FIXMSZIP (0)
#define PARAM_DECOMPBUF (65536)
	
	//int internal_error;
	assert ((ct & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
/*	switch (ct & cffoldCOMPTYPE_MASK) {
		case cffoldCOMPTYPE_MSZIP:*/
	state = new mszipd_stream(&decsys, fh, fh, PARAM_DECOMPBUF, PARAM_FIXMSZIP);
/*			break;
		default:
			return internal_error = MSPACK_ERR_DATAFORMAT;
	}*/
	return MSPACK_ERR_OK;
	//return internal_error = (state) ? MSPACK_ERR_OK : MSPACK_ERR_NOMEMORY;
}

int dec_system::read(struct PackFile *file, void *buffer, int bytes) {
	mscab_decompressor *handle = (struct mscab_decompressor *) file;
	unsigned char *buf = (unsigned char *) buffer;
	//mspack_system *sys = handle->getSystem();
	int avail, todo, outlen = 0, ignore_cksum;
	
	ignore_cksum = PARAM_FIXMSZIP && ((handle->d->comp_type & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);
	
	todo = bytes;
	while (todo > 0) {
		avail = handle->d->i_end - handle->d->i_ptr;
		
		if (avail) {
			if (avail > todo) avail = todo;
			memcpy(buf, handle->d->i_ptr, (size_t) avail);
			handle->d->i_ptr += avail;
			buf  += avail;
			todo -= avail;
		}
		else {
			if (handle->d->block++ >= handle->d->folder->num_blocks) {
				handle->error = MSPACK_ERR_DATAFORMAT;
				break;
			}
			
			handle->error = read_block(handle->d, &outlen, ignore_cksum);
			if (handle->error) return -1;
			
			if (handle->d->block >= handle->d->folder->num_blocks) {
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
	handle->d->offset += bytes;
	if (handle->d->outfh) {
		return handle->d->outfh->write(buffer, bytes);
	}
	return bytes;
}

int dec_system::read_block(mscabd_decompress_state *d, int *out, int ignore_cksum) {
	unsigned char hdr[cfdata_SIZEOF];
	unsigned int cksum;
	int len;
	
	d->i_ptr = d->i_end = &d->input[0];
	
	do {
		if (d->infh->read(&hdr[0], cfdata_SIZEOF) != cfdata_SIZEOF) {
			return MSPACK_ERR_READ;
		}
		
		if (d->data->cab->block_resv &&
			d->infh->seek((off_t) d->data->cab->block_resv, PackFile::SEEKMODE_CUR))
		{
			return MSPACK_ERR_SEEK;
		}
		
		len = EndGetI16(&hdr[cfdata_CompressedSize]);
		if (((d->i_end - d->i_ptr) + len) > CAB_INPUTMAX) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		if (EndGetI16(&hdr[cfdata_UncompressedSize]) > CAB_BLOCKMAX) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		if (d->infh->read(d->i_end, len) != len) {
			return MSPACK_ERR_READ;
		}
		
		if ((cksum = EndGetI32(&hdr[cfdata_CheckSum]))) {
			unsigned int sum2 = cabd_checksum(d->i_end, (unsigned int) len, 0);
			if (cabd_checksum(&hdr[4], 4, sum2) != cksum) {
				if (!ignore_cksum) return MSPACK_ERR_CHECKSUM;
				fprintf(stderr, "WARNING; bad block checksum found");
			}
		}
		
		d->i_end += len;
		
		if ((*out = EndGetI16(&hdr[cfdata_UncompressedSize]))) {
			return MSPACK_ERR_OK;
		}
		
		//sys->close(d->infh);
		delete d->infh;
		d->infh = NULL;
		
		if (!(d->data = d->data->next)) {
			return MSPACK_ERR_DATAFORMAT;
		}
		
		d->incab = d->data->cab;
		if (!(d->infh = new PackFile(d->incab->GetFilename(), PackFile::OPEN_READ)))
		{
			return MSPACK_ERR_OPEN;
		}
		
		if (d->infh->seek(d->data->offset, PackFile::SEEKMODE_START)) {
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
	filename_size = file->filename.length();
	
	//Skip executables and libries
	const char *ext = file->filename.substr(filename_size - 3).c_str();
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
	if (filename_size > 3 &&  file->filename[2] == '_') {
		//printf("Grim-style\n");
		char file_lang[3];
		sscanf(file->filename.c_str(), "%2s_%s",file_lang, filename);
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
	if ((filename_size > lcode_size_com && strncmp(file->filename.c_str(), LANG_ALL2, lcode_size_com - 1) == 0) ||
	    (filename_size > lcode_size_loc && strncmp(file->filename.c_str(), kLanguages_code2[lang], lcode_size_loc) == 0) ) {
		char *fn = rindex(file->filename.c_str(), '\\') + 1;
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
