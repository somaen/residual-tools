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
#include <common/endian.h>
#include "3do2obj.h"

// Multi-way converter for 3DO-files, currently very WIP.

void Vector2d::set(float x, float y) {
	_coords[0] = x;
	_coords[1] = y;
}
Vector2d::Vector2d(float x, float y) {
	set(x,y);
}
// TODO: Proper operator
std::string Vector2d::toString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << _coords[0] << "\t" << _coords[1];
	
	return ss.str();
}

std::string Vector2d::toOBJString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << _coords[0] << " " << _coords[1];
	
	return ss.str();
}

void Vector3d::set(float x, float y, float z) {
	_coords[0] = x;
	_coords[1] = y;
	_coords[2] = z;
}

Vector3d::Vector3d(float x, float y, float z) {
	set(x,y,z);
}
// TODO: Proper operator
std::string Vector3d::toString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << _coords[0] << "\t" << _coords[1] << "\t" << _coords[2];
	
	return ss.str();
}

std::string Vector3d::toOBJString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << _coords[0] << " " << _coords[1] << " " << _coords[2];
	
	return ss.str();
}


inline Vector3d get_vector3d(const char *data) {
	return Vector3d(get_float(data), get_float(data + 4), get_float(data + 8));
};

inline Vector2d get_vector2d(const char *data) {
	return Vector2d(get_float(data), get_float(data + 4));
};



int MeshFace::loadBinary(const char *&data) {
	_type = READ_LE_UINT32(data + 4);
	_geo = READ_LE_UINT32(data + 8);
	_light = READ_LE_UINT32(data + 12);
	_tex = READ_LE_UINT32(data + 16);
	_numVertices = READ_LE_UINT32(data + 20);
	int texPtr = READ_LE_UINT32(data + 28);
	int materialPtr = READ_LE_UINT32(data + 32);
	_extraLight = get_float(data + 48);
	_normal = get_vector3d(data + 64);
	data += 76;
	
	_vertices = new int[_numVertices];
	for (uint i = 0; i < _numVertices; i++) {
		_vertices[i] = READ_LE_UINT32(data);
		data += 4;
	}
	if (texPtr == 0)
		_texVertices = NULL;
	else {
		_texVertices = new int[_numVertices];
		for (uint i = 0; i < _numVertices; i++) {
			_texVertices[i] = READ_LE_UINT32(data);
			data += 4;
		}
	}
	if (materialPtr == 0)
		_materialIndex = 0;
	else {
		//_material = materials[READ_LE_UINT32(data)];
		_materialIndex = READ_LE_UINT32(data);
		materialPtr = READ_LE_UINT32(data);
		data += 4;
	}
	return materialPtr;
}

std::string MeshFace::to3DOString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(4);
	// TODO: type is 4-character hex, extralight is 4-decimal float
	ss << _materialIndex << "\t0x";
	ss << std::hex << std::setw(4) << std::setfill('0') << _type << std::dec;
	ss << "\t" << _geo << "\t" << _light << "\t" << _tex << "\t" << _extraLight << "\t" << _numVertices;
	// TODO, this part has fixed-point comma.
	for(uint i = 0; i < _numVertices; i++) {
		ss << "\t" << _vertices[i] << ", " << _texVertices[i];
	}
	
	return ss.str();
}

std::string MeshFace::toOBJString(uint vertOffset, uint texOffset) {
	// TODO, actually fix all this stuff properly.
	std::stringstream ss;
	//ss << std::fixed;
	//ss << std::setprecision(4);
	// TODO: type is 4-character hex, extralight is 4-decimal float
	//ss << _materialIndex << "\t0x";
	//ss << std::hex << std::setw(4) << std::setfill('0') << _type << std::dec;
	//ss << "\t" << _geo << "\t" << _light << "\t" << _tex << "\t" << _extraLight << "\t" << _numVertices;
	// TODO, this part has fixed-point comma.
	ss << "f";
	for(uint i = 0; i < _numVertices; i++) {
		ss << " " << _vertices[i]+1+vertOffset ;//<< "/" << _texVertices[i]+1 << "/" << _vertices[i]+1;
	}
	
	return ss.str();
}

// This should be where the magic actually happens.
void Mesh::loadBinary(const char *&data) {
	// 32 bytes with mesh-name
	memcpy(_name, data, 32);
	data+=36;
	
	_geometryMode = READ_LE_UINT32(data);
	_lightingMode = READ_LE_UINT32(data + 4);
	_textureMode = READ_LE_UINT32(data + 8);
	_numVertices = READ_LE_UINT32(data + 12);
	_numTextureVerts = READ_LE_UINT32(data + 16);
	_numFaces = READ_LE_UINT32(data + 20);
	
	_vertices = new Vector3d[_numVertices];
	_verticesI = new float[_numVertices];
	_vertNormals = new Vector3d[_numVertices];
	_textureVerts = new Vector2d[_numTextureVerts];
	_faces = new MeshFace[_numFaces];
	_materialID = new uint[_numFaces];
	data += 24;
	/*for (uint i = 0; i < 3 * _numVertices; i++) {
	 // TODO: Use Vector3d for this, to simplify storage.
	 _vertices[i] = get_float(data);
	 data += 4;
	 }*/
	for (uint i = 0; i < _numVertices; i++) {
		// TODO: Use Vector3d for this, to simplify storage.
		_vertices[i] = get_vector3d(data);
		data += 12;
	}
	for (uint i = 0; i < _numTextureVerts; i++) {
		_textureVerts[i] = get_vector2d(data);
		data += 8;
	}
	for (uint i = 0; i < _numVertices; i++) {
		_verticesI[i] = get_float(data);
		data += 4;
	}
	data += _numVertices * 4;
	for (uint i = 0; i < _numFaces; i++)
		//_materialid[i] = _faces[i].loadBinary(data, materials);
		_materialID[i] = _faces[i].loadBinary(data);
	for (uint i = 0; i < _numVertices; i++) {
		_vertNormals[i] = get_vector3d(data);
		data += 12;
	}
	_shadow = READ_LE_UINT32(data);
	_radius = get_float(data + 8);
	data += 36;
}

std::string Mesh::writeOBJText(uint vertOffset, uint texOffset) {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << "#NAME " << _name << std::endl;
	ss << "#RADIUS\t" << _radius << std::endl
	<< "#SHADOW\t" << _shadow << std::endl
	<< "#GEOMETRYMODE\t" << _geometryMode << std::endl
	<< "#LIGHTINGMODE\t" << _lightingMode << std::endl
	<< "#TEXTUREMODE\t" << _textureMode << std::endl;
	
	ss << "#VERTICES " << _numVertices << std::endl;
	for(uint i = 0; i < _numVertices; i++) {
		ss << "v " << _vertices[i].toOBJString() << std::endl;
	}
	ss << "#TEXTURE VERTICES " << _numTextureVerts << std::endl;
	for(uint i = 0; i < _numTextureVerts; i++) {
		ss << "vt\t" << _textureVerts[i].toString() << std::endl;
	}
	ss << "#VERTEX NORMALS" << std::endl;
	for(uint i = 0; i < _numVertices; i++) {
		ss << "vn" << "\t" << _vertNormals[i].toString() << std::endl;
	}
	
	ss << "#FACES " << _numFaces << std::endl;
	ss << "o " << _name << std::endl;
	for(uint i = 0; i < _numFaces; i++) {
		ss << _faces[i].toOBJString(vertOffset,texOffset) << std::endl;
	}
	// TODO, fix this for OBJ
	ss << "#FACE NORMALS" << std::endl;
	for(uint i = 0; i < _numFaces; i++) {
		ss << "#\t" << i << ":\t" << _faces[i]._normal.toString() << std::endl;
	}
	
	return ss.str();	
}


std::string Mesh::write3DOText() {
	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(6);
	ss << "NAME " << _name << std::endl;
	ss << "RADIUS\t" << _radius << std::endl
	<< "SHADOW\t" << _shadow << std::endl
	<< "GEOMETRYMODE\t" << _geometryMode << std::endl
	<< "LIGHTINGMODE\t" << _lightingMode << std::endl
	<< "TEXTUREMODE\t" << _textureMode << std::endl;
	
	ss << "VERTICES " << _numVertices << std::endl;
	for(uint i = 0; i < _numVertices; i++) {
		ss << "\t" << i << ":\t" << _vertices[i].toString() << "\t" << 0.0f << std::endl;
	}
	ss << "TEXTURE VERTICES " << _numTextureVerts << std::endl;
	for(uint i = 0; i < _numTextureVerts; i++) {
		ss << "\t" << i << ":\t" << _textureVerts[i].toString() << std::endl;
	}
	ss << "VERTEX NORMALS" << std::endl;
	for(uint i = 0; i < _numVertices; i++) {
		ss << "\t" << i << ":\t" << _vertNormals[i].toString() << "\t" << 0.0f << std::endl;
	}
	
	ss << "FACES " << _numFaces << std::endl;
	for(uint i = 0; i < _numFaces; i++) {
		ss << "\t" << i << ":\t" << _faces[i].to3DOString() << std::endl;
	}
	
	ss << "FACE NORMALS" << std::endl;
	for(uint i = 0; i < _numFaces; i++) {
		ss << "\t" << i << ":\t" << _faces[i]._normal.toString() << std::endl;
	}
	
	return ss.str();
	
}

void Geoset::loadBinary(const char *&data) {
	_numMeshes = READ_LE_UINT32(data);
	data+=4;
	_meshes = new Mesh[_numMeshes];
	for (uint i = 0; i < _numMeshes; i++)
		//_meshes[i].loadBinary(data, materials);
		_meshes[i].loadBinary(data);
}

std::string Geoset::write3DOText() {
	std::stringstream ss;
	ss << "MESHES " << _numMeshes << std::endl;
	for(uint i = 0; i < _numMeshes; i++) {
		ss << "MESH " << i << std::endl;
		ss << _meshes[i].write3DOText();
	}
	return ss.str();
}

std::string Geoset::writeOBJText() {
	std::stringstream ss;
	uint texOffset = 0;
	uint vertOffset = 0;
	ss << "#MESHES " << _numMeshes << std::endl;
	for(uint i = 0; i < _numMeshes; i++) {
		ss << "#MESH " << i << std::endl;
		ss << _meshes[i].writeOBJText(vertOffset, texOffset);
		
		// OBJs keep a running count, while 3DOs keep a per-mesh count.
		// This WON'T work if there are multiple geosets in a file.
		texOffset += _meshes[i]._numTextureVerts;
		vertOffset += _meshes[i]._numVertices;
	}
	return ss.str();
}


void ModelNode::loadBinary(const char *&data, ModelNode *hierNodes, const Geoset *g) {
	memcpy(_name, data, 64);
	_flags = READ_LE_UINT32(data + 64);
	_type = READ_LE_UINT32(data + 72);
	_meshNum = READ_LE_UINT32(data + 76);
	if (_meshNum < 0)
		_mesh = NULL;
	else
		_mesh = g->_meshes + _meshNum;
	_depth = READ_LE_UINT32(data + 80);
	int parentPtr = READ_LE_UINT32(data + 84);
	_numChildren = READ_LE_UINT32(data + 88);
	int childPtr = READ_LE_UINT32(data + 92);
	int siblingPtr = READ_LE_UINT32(data + 96);
	_pivot = get_vector3d(data + 100);
	_pos = get_vector3d(data + 112);
	_pitch = get_float(data + 124);
	_yaw = get_float(data + 128);
	_roll = get_float(data + 132);
	_animPos.set(0,0,0);
	_animPitch = 0;
	_animYaw = 0;
	_animRoll = 0;
	//_sprite = NULL;
	
	data += 184;
	
	if (parentPtr != 0) {
		_parentIdx = READ_LE_UINT32(data);
		_parent = hierNodes + _parentIdx;
		data += 4;
	} else
		_parent = NULL;
	if (childPtr != 0) {
		_childIdx = READ_LE_UINT32(data);
		_child = hierNodes + _childIdx;
		data += 4;
	} else
		_child = NULL;
	if (siblingPtr != 0) {
		_siblingIdx = READ_LE_UINT32(data);
		_sibling = hierNodes + _siblingIdx;
		data += 4;
	} else
		_sibling = NULL;
	
	_meshVisible = true;
	_hierVisible = true;
	_initialized = true;
}

std::string ModelNode::toString() {
	std::stringstream ss;
	ss << std::setfill('0');
	ss << "0x" << std::hex << std::setw(4) << _flags << "\t";
	ss << "0x" << std::hex << std::setw(5) << _type << "\t";
	ss << std::dec;
	
	ss << _meshNum << "\t" << _parentIdx << "\t" << _childIdx << "\t" << _siblingIdx << "\t" << _numChildren << "\t";
	
	ss << _pos.toString() << "\t" << _pitch << "\t" << _yaw << "\t" << _roll << "\t" <<
	_pivot.toString() << "\t" << _name;
	
	return ss.str();
}

void Model::loadText() {
	
}

void Model::loadBinary(const char *&data, uint length) {
	_id = READ_LE_UINT32(data);
	// Num-materials
	_numMaterials = READ_LE_UINT32(data+4);
	data += 8;
	// Each material has 32 bytes reserved for it's name.
	// We don't really care about the materials, just their names.
	_materialNames = new char[_numMaterials][32];
	// TODO: Read materialNames
	for(uint i = 0; i < _numMaterials; i++) {
		strcpy(_materialNames[i], data);
		data += 32;
	}
	// Then follows the 3do-name
	memcpy(_modelName, data, 32);
	data += 36; // Skip 4 bytes after the name.
				// Num-geosets
	_numGeosets = READ_LE_UINT32(data);
	_geoSets = new Geoset[_numGeosets];
	data += 4;
	for(uint i = 0; i < _numGeosets; i++) {
		_geoSets[i].loadBinary(data);
	}
	_numHierNodes = READ_LE_UINT32(data + 4);
	data += 8;
	_rootHierNode = new ModelNode[_numHierNodes];
	for (uint i = 0; i < _numHierNodes; i++) {
		_rootHierNode[i].loadBinary(data, _rootHierNode, &_geoSets[0]);
	}
	_radius = get_float(data);
	_insertOffset = get_vector3d(data + 40);
}

// Savers

std::string Model::write3DOText() {
	std::stringstream ss;
	
	// Header:
	ss << "# MODEL " << _modelName << " exported by 3do2obj" << std::endl;
	
	// Skipping the comment-blocks, we get:
	ss << "SECTION: HEADER" << std::endl;
	ss << "3DO 2.1" << std::endl;
	
	ss << "SECTION: MODELRESOURCE" << std::endl;
	ss << "MATERIALS " << _numMaterials << std::endl;
	for(uint i = 0; i < _numMaterials; i++) {
		ss << "\t\t" << i << ":" << "\t" << _materialNames[i] << std::endl;
	}
	
	ss << "SECTION: GEOMETRYDEF" << std::endl;
	ss << "RADIUS\t" << _radius << std::endl; // TODO: Set float-precision to 6 decimals, padded.
	// TODO: Add vector2stream-operator.
	ss << "INSERT OFFSET\t" << _insertOffset._coords[0] << "\t" << _insertOffset._coords[0] << "\t" <<
	_insertOffset._coords[0] << "\t" << std::endl;
	
	ss << "GEOSETS " << _numGeosets << std::endl;
	for(uint i = 0; i < _numGeosets; i++) {
		ss << "GEOSET " << i << std::endl;
		ss << _geoSets[i].write3DOText();
	}
	
	ss << "SECTION: HIERARCHYDEF" << std::endl;
	// This is a tree, but it's stored in an array, so just print it.
	ss << "HIERARCHY NODES " << _numHierNodes << std::endl;
	for(uint i = 0; i < _numHierNodes; i++) {
		ss << "\t" << i << ":\t" << _rootHierNode[i].toString() << std::endl;
	}
	return ss.str();
}

std::string Model::writeOBJText() {
	std::stringstream ss;
	// TODO: Add materials.
	ss << "# Materials not currently included" << std::endl;
	
	ss << "# Geosets" << std::endl;
	if (_numGeosets > 1)
		ss << "# Warning, more than 1 geoset, face-offsets will be off" << std::endl;
	for(uint i = 0; i < _numGeosets; i++) {
		ss << "#GEOSET " << i << std::endl;
		ss << _geoSets[i].writeOBJText();
	} 
	
	return ss.str();
}


Model *ParseFile(const char *data, uint length, std::string filename) {
	if(data[0] == '#') {
		std::cout << "Text-loading not implemented";
		assert(0);
	}
	uint id;
	//char *materialNames = new char[numMaterials][32];
	id = READ_LE_UINT32(data);
	//file.read((char *)&id, 4);
	assert(id=='MODL');
	
	Model *m = new Model();
	m->loadBinary(data,length);
	return m;
}	

std::string PrintAsOBJ() {
	std::stringstream ss;
	
	return ss.str();
}

int main(int argc, char **argv) {
	std::string filename=argv[1];
	outputType outType;
	
	std::fstream file(argv[1], std::fstream::in|std::fstream::binary);
	if (!file.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return 0;
	}
	// TODO: Make this select based on the input-data, or the parameter, properly.
	if (argc == 2)
		outType = TEXT_3DO;
	else if (argc == 3)
		outType = WAVEFORM_OBJ;
	
	std::string outname = argv[1];
	outname += ".parsed";
	
	file.seekg(0, std::ios::end);
	int end = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	char *data = new char[end];
	file.read(data, end);
	file.close();
	
	Model *m = ParseFile(data, end, outname);
	if (outType == TEXT_3DO)
		std::cout << m->write3DOText();
	else if (outType == WAVEFORM_OBJ)
		std::cout << m->writeOBJText();
	return 0;
}