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

int main(int argc, char **argv) {
	std::string filename=argv[1];

	float texFactor = 4.0f;
	std::fstream file(argv[1], std::fstream::in|std::fstream::binary);
	if (!file.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return 0;
	}
	
	std::string outname = argv[1];
	outname += ".parsed";
	
	file.seekg(0, std::ios::end);
	int end = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	char *data = new char[end];
	file.read(data, end);
	file.close();
	
	parse3DO(data, end, texFactor);
	
	file.open(outname.c_str(), std::fstream::out|std::fstream::binary);
	file.write(data,end);
	
	return 0;
}
