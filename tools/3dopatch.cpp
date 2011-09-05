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
 * $URL:
 * $Id:
 *
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <common/endian.h>
#include "3dopatch.h"

// Multi-way converter for 3DO-files, currently very WIP.
// Borrows heavily from model.cpp in Residual

void Vector2d::set(float x, float y) {
	_coords[0] = x;
	_coords[1] = y;
}
Vector2d::Vector2d(float x, float y) {
	set(x,y);
}

Vector2d get_vector2d(const char *data) {
	return Vector2d(get_float(data), get_float(data + 4));
};

struct lab_header {
	uint32_t magic;
	uint32_t magic2;
	uint32_t num_entries;
	uint32_t string_table_size;
	uint32_t string_table_offset;
};

struct lab_entry {
	uint32_t fname_offset;
	uint32_t start;
	uint32_t size;
	uint32_t reserved;
};

void parse3DO(char *data, int length, float texFactor) {
	//char *materialNames = new char[numMaterials][32];
	int id = READ_LE_UINT32(data);
	//file.read((char *)&id, 4);
	assert(id=='MODL');
	
	// Num-materials
	int numMaterials = READ_LE_UINT32(data+4);
	data += 8;
	// Each material has 32 bytes reserved for it's name.
	// We don't really care about the materials, just their names.
	for(int i = 0; i < numMaterials; i++) {
		data += 32;
	}
	data += 36; // Skip 4 bytes after the name.
				// Num-geosets
	int numGeosets = READ_LE_UINT32(data);
	data += 4;
	for(int i = 0; i < numGeosets; i++) {
		int numMeshes = READ_LE_UINT32(data);
		data+=4;
		for (int i = 0; i < numMeshes; i++) {
			// 32 bytes with mesh-name
			data+=36;
			int numVertices = READ_LE_UINT32(data + 12);
			int numTextureVerts = READ_LE_UINT32(data + 16);
			int numFaces = READ_LE_UINT32(data + 20);
			
			Vector2d textureVert;
			data += 24;

			for (int i = 0; i < numVertices; i++) {
				data += 12;
			}
			for (int i = 0; i < numTextureVerts; i++) {
				textureVert = get_vector2d(data);
				textureVert._coords[0] = textureVert._coords[0] * texFactor;
				textureVert._coords[1] = textureVert._coords[1] * texFactor;
				memcpy(data, &textureVert._coords[0], 4);
				memcpy(data + 4, &textureVert._coords[1], 4);
				data += 8;
			}
			for (int i = 0; i < numVertices; i++) {
				data += 4;
			}
			data += numVertices * 4;
			for (int i = 0; i < numFaces; i++) {
				int numVertices = READ_LE_UINT32(data + 20);
				int texPtr = READ_LE_UINT32(data + 28);
				int materialPtr = READ_LE_UINT32(data + 32);
				data += 76;
				
				for (int i = 0; i < numVertices; i++) {
					data += 4;
				}
				
				if (texPtr != 0) {
					for (int i = 0; i < numVertices; i++) {
						data += 4;
					}
				}
				
				if (materialPtr != 0) {
					data += 4;
				}
			}
			for (int i = 0; i < numVertices; i++) {
				data += 12;
			}
			data += 36;
		}
	}
	data += 8;
}

#define GT_GRIM 1
#define GT_EMI 2

int getLabOffset(std::string labname, std::string filename, int& length) {
	FILE *infile;
	struct lab_header head;
	struct lab_entry *entries;
	char *str_table;
	uint32_t i;
	uint32_t offset;
	uint8_t g_type;
	
	infile = fopen(labname.c_str(), "rb");
	if (infile == 0) {
		printf("Can not open source file: %s\n", filename.c_str());
		exit(1);
	}
	
	fread(&head.magic, 1, 4, infile);
	fread(&head.magic2, 1, 4, infile);
	uint32_t num, s_size, s_offset;
	fread(&num, 1, 4, infile);
	fread(&s_size, 1, 4, infile);
	
	uint32_t typeTest = 0;
	fread(&typeTest,1,4,infile); 
	if (typeTest == 0) { // First entry of the table has offset 0 for Grim
		g_type = GT_GRIM;
		fseek(infile, -4, SEEK_CUR);
	} else { // EMI has an offset instead.
		s_offset = typeTest;
		g_type = GT_EMI;
	}
	head.num_entries = READ_LE_UINT32(&num);
	head.string_table_size = READ_LE_UINT32(&s_size);
	if (0 != memcmp(&head.magic, "LABN", 4)) {
		printf("There is no LABN header in source file\n");
		exit(1);
	}
	
	entries = (struct lab_entry *)malloc(head.num_entries * sizeof(struct lab_entry));
	str_table = (char *)malloc(head.string_table_size);
	if (!str_table || !entries) {
		printf("Could not allocate memory\n");
		exit(1);
	}
	// Grim-stuff
	if(g_type == GT_GRIM) {
		fread(entries, 1, head.num_entries * sizeof(struct lab_entry), infile);
		
		fread(str_table, 1, head.string_table_size, infile);
	} else if(g_type == GT_EMI) { // EMI-stuff
								  // EMI has a string-table-offset
		head.string_table_offset = READ_LE_UINT32(&s_offset) - 0x13d0f;
		// Find the string-table
		fseek(infile, head.string_table_offset, SEEK_SET);
		// Read the entire string table into str-table
		fread(str_table, 1, head.string_table_size, infile);
		fseek(infile, 20, SEEK_SET);
		
		// Decrypt the string table
		uint32_t j;
		for (j = 0; j < head.string_table_size; j++)
			if (str_table[j] != 0)
				str_table[j] ^= 0x96;
		fread(entries, 1, head.num_entries * sizeof(struct lab_entry), infile);
		
	}
	// allocate a 1mb buffer to start with
	uint32_t bufSize = 1024*1024;
	char *buf = (char *)malloc(bufSize);
	if (!buf) {
		printf("Could not allocate memory\n");
		exit(1);
	}
	for (i = 0; i < head.num_entries; i++) {
		const char *fname = str_table + READ_LE_UINT32(&entries[i].fname_offset);
		std::string outfilename(fname);
		if (outfilename != filename)
			continue;
		//outfile = fopen(fname, "wb");
	/*	if (!outfile) {
			printf("Could not open file: %s\n", fname);
			continue;
		}*/
		offset = READ_LE_UINT32(&entries[i].start);
		uint32_t size = READ_LE_UINT32(&entries[i].size);
	/*	if (bufSize < size) {
			bufSize = size;
			char *newBuf = (char *)realloc(buf, bufSize);
			if (!newBuf) {
				printf("Could not reallocate memory\n");
				exit(1);
			} else {
				buf = newBuf;
			}
		}*/
		length = size;
		free(buf);
		free(entries);
		free(str_table);
		fclose(infile);
		return offset;
/*		fseek(infile, offset, SEEK_SET);
		fread(buf, 1, READ_LE_UINT32(&entries[i].size), infile);
		fwrite(buf, 1, READ_LE_UINT32(&entries[i].size), outfile);
		fclose(outfile);*/
		
	}
	free(buf);
	free(entries);
	free(str_table);
	
	fclose(infile);
	return -1;
}

int main(int argc, char **argv) {
	std::string filename=argv[1];

	float texFactor = 4.0f;
	
	int length = 0;
	
	std::fstream *file;
	if (argc < 3) {
		file = new std::fstream(argv[1], std::fstream::in|std::fstream::binary);
		file->seekg(0, std::ios::end);
		length = (int)file->tellg();
		file->seekg(0, std::ios::beg);
		char *data = new char[length];
		file->read(data, length);
		file->close();
		std::string outname = argv[1];
		outname += ".parsed";
		
		parse3DO(data, length, texFactor);
		
		file->open(outname.c_str(), std::fstream::out|std::fstream::binary);
		file->write(data,length);
		
		file->close();
		
		delete[] data;
	} else {
		file = new std::fstream(argv[1], std::fstream::in|std::fstream::binary);
		int offset = getLabOffset(argv[1], argv[2], length);
		file->seekg(offset, std::ios::beg);
		char *data = new char[length];
		file->read(data, length);
		file->close();
		
		parse3DO(data, length, texFactor);
		
		file->open(argv[1], std::fstream::out|std::fstream::binary);
		file->seekp(offset, std::ios::beg);
		file->write(data,length);
		
		file->close();
		
		delete[] data;
	}
	if (!file->is_open()) {
		std::cout << "Could not open file" << std::endl;
		return 0;
	}
	
	
	return 0;
}
