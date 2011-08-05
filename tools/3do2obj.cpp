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
#include <string>
#include <fstream>
#include <common/endian.h>

class Mesh;
class MeshFace;
class Geoset;
class Model;

struct Vector3d {
	float _coords[3];
	void set(float x, float y, float z) {
		_coords[0] = x;
		_coords[1] = y;
		_coords[2] = z;
	}
	Vector3d(float x=0.0f, float y=0.0f, float z=0.0f) {
		set(x,y,z);
	}
};

inline Vector3d get_vector3d(const char *data) {
	return Vector3d(get_float(data), get_float(data + 4), get_float(data + 8));
};


struct MeshFace
{
	//Material *_material;
	uint _materialIndex;
	int _type, _geo, _light, _tex;
	float _extraLight;
	int _numVertices;
	int *_vertices, *_texVertices;
	Vector3d _normal;
	int loadBinary(const char *&data) {
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
		for (int i = 0; i < _numVertices; i++) {
			_vertices[i] = READ_LE_UINT32(data);
			data += 4;
		}
		if (texPtr == 0)
			_texVertices = NULL;
		else {
			_texVertices = new int[_numVertices];
			for (int i = 0; i < _numVertices; i++) {
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
	
};

struct Mesh
{
	char _name[32];
	uint _geometryMode, _lightingMode, _textureMode, _numVertices, _numTextureVerts, _numFaces, _shadow;
	float *_vertices;
	float *_verticesI;
	float *_vertNormals;
	float *_textureVerts;
	float _radius;
	MeshFace *_faces;
	uint *_materialID;
	// This should be where the magic actually happens.
	void loadBinary(const char *&data) {
		// 32 bytes with mesh-name
		memcpy(_name, data, 32);
		
		_geometryMode = READ_LE_UINT32(data + 36);
		_lightingMode = READ_LE_UINT32(data + 40);
		_textureMode = READ_LE_UINT32(data + 44);
		_numVertices = READ_LE_UINT32(data + 48);
		_numTextureVerts = READ_LE_UINT32(data + 52);
		_numFaces = READ_LE_UINT32(data + 56);

		_vertices = new float[3 * _numVertices];
		_verticesI = new float[_numVertices];
		_vertNormals = new float[3 * _numVertices];
		_textureVerts = new float[2 * _numTextureVerts];
		_faces = new MeshFace[_numFaces];
		_materialID = new uint[_numFaces];
		data += 60;
		for (uint i = 0; i < 3 * _numVertices; i++) {
			_vertices[i] = get_float(data);
			data += 4;
		}
		for (uint i = 0; i < 2 * _numTextureVerts; i++) {
			_textureVerts[i] = get_float(data);
			data += 4;
		}
		for (uint i = 0; i < _numVertices; i++) {
			_verticesI[i] = get_float(data);
			data += 4;
		}
		data += _numVertices * 4;
		for (uint i = 0; i < _numFaces; i++)
			//_materialid[i] = _faces[i].loadBinary(data, materials);
			_materialID[i] = _faces[i].loadBinary(data);
		for (uint i = 0; i < 3 * _numVertices; i++) {
			_vertNormals[i] = get_float(data);
			data += 4;
		}
		_shadow = READ_LE_UINT32(data);
		_radius = get_float(data + 8);
		data += 36;
	}
};

class Geoset
{
public:
	uint _numMeshes;
	Mesh *_meshes;
	void loadBinary(const char *data) {
		_numMeshes = READ_LE_UINT32(data);
		data+=8;
		_meshes = new Mesh[_numMeshes];
		for (uint i = 0; i < _numMeshes; i++)
			//_meshes[i].loadBinary(data, materials);
			_meshes[i].loadBinary(data);
	}
};

struct ModelNode {
	char _name[64];
	Mesh *_mesh;
	int _flags, _type;
	int _depth, _numChildren;
	ModelNode *_parent, *_child, *_sibling;
	Vector3d _pos, _pivot;
	float _pitch, _yaw, _roll;
	Vector3d _animPos;
	float _animPitch, _animYaw, _animRoll;
	bool _meshVisible, _hierVisible;
	bool _initialized;
	//Graphics::Matrix4 _matrix;
	//Graphics::Matrix4 _localMatrix;
	//Graphics::Matrix4 _pivotMatrix;
	//Sprite* _sprite;
	
	
	void loadBinary(const char *&data, ModelNode *hierNodes, const Geoset *g) {
		memcpy(_name, data, 64);
		_flags = READ_LE_UINT32(data + 64);
		_type = READ_LE_UINT32(data + 72);
		int meshNum = READ_LE_UINT32(data + 76);
		if (meshNum < 0)
			_mesh = NULL;
		else
			_mesh = g->_meshes + meshNum;
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
			_parent = hierNodes + READ_LE_UINT32(data);
			data += 4;
		} else
			_parent = NULL;
		if (childPtr != 0) {
			_child = hierNodes + READ_LE_UINT32(data);
			data += 4;
		} else
			_child = NULL;
		if (siblingPtr != 0) {
			_sibling = hierNodes + READ_LE_UINT32(data);
			data += 4;
		} else
			_sibling = NULL;
		
		_meshVisible = true;
		_hierVisible = true;
		_initialized = true;
	}
};


class Model {
public:
	uint32 _id;
	uint32 _numMaterials, _numGeosets, _numHierNodes;
	Geoset *_geoSets;
	ModelNode *_rootHierNode;
	Vector3d _insertOffset;
	float _radius;
	char _modelName[32]; 
	char (*_materialNames)[32];
	
	void loadBinary(const char *&data, uint length) {
		_id = READ_LE_UINT32(data);
		// Num-materials
		_numMaterials = READ_LE_UINT32(data+4);
		// Each material has 32 bytes reserved for it's name.
		// We don't really care about the materials, just their names.
		_materialNames = new char[_numMaterials][32];
		// TODO: Read materialNames
		for(uint i = 0; i < _numMaterials; i++) {
			strcpy(_materialNames[i], data);
			data += 32;
		}
		// Let's adjust the data-pointer, to the offset after that block, to simplify offsets hereafter.
		data +=8 + _numMaterials * 32;
		// Then follows the 3do-name
		memcpy(_modelName, data, 32);
		
		// Num-geosets
		_numGeosets = READ_LE_UINT32(data + 32);
		_geoSets = new Geoset[_numGeosets];
		data += 36;
		for(uint i = 0; i < length; i++) {
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
};

void ParseFile(const char *data, uint length, std::string filename) {
	
	uint id;
	//char *materialNames = new char[numMaterials][32];
	id = READ_LE_UINT32(data);
	//file.read((char *)&id, 4);
	assert(id=='MODL');
	
	Model m;
	m.loadBinary(data,length);
}	

int main(int argc, char **argv) {
	std::string filename=argv[1];
	
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
	
	ParseFile(data, end, outname);

	return 0;
}