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

#ifndef _3do2obj_h
#define _3do2obj_h

#include <string>
#include <iostream>

// Multi-way converter for 3DO-files, currently very WIP.
// Borrows heavily from model.cpp in Residual

enum outputType {
	TEXT_3DO = 1,
	BINARY_3DO = 2,
	WAVEFORM_OBJ = 3
};

struct Vector2d {
	float _coords[2];
	void set(float x, float y);
	Vector2d(float x=0.0f, float y=0.0f);
	// TODO: Proper operator
	std::string toString();
	std::string toOBJString();
};

struct Vector3d {
	float _coords[3];
	void set(float x, float y, float z);
	Vector3d(float x=0.0f, float y=0.0f, float z=0.0f);
	// TODO: Proper operator
	std::string toString();
	std::string toOBJString();
};


struct MeshFace
{
	//Material *_material;
	uint _materialIndex;
	int _type, _geo, _light, _tex;
	float _extraLight;
	uint _numVertices;
	int *_vertices, *_texVertices;
	Vector3d _normal;
	int loadBinary(const char *&data);	
	std::string to3DOString();
	std::string toOBJString(uint, uint);	
};

struct Mesh
{
	char _name[32];
	uint _geometryMode, _lightingMode, _textureMode, _numVertices, _numTextureVerts, _numFaces, _shadow;
	//float *_vertices;
	Vector3d *_vertices;
	float *_verticesI;
	Vector3d *_vertNormals;
	Vector2d *_textureVerts;
	float _radius;
	MeshFace *_faces;
	uint *_materialID;
	// This should be where the magic actually happens.
	void loadBinary(const char *&data);	
	std::string write3DOText();
	std::string writeOBJText(uint, uint);
};

class Geoset
{
public:
	uint _numMeshes;
	Mesh *_meshes;
	void loadBinary(const char *&data);
	std::string write3DOText();
	std::string writeOBJText();
};

struct ModelNode {
	char _name[64];
	Mesh *_mesh;
	int _flags, _type;
	int _depth, _numChildren;
	ModelNode *_parent, *_child, *_sibling;
	int _parentIdx, _childIdx, _siblingIdx;
	int _meshNum;
	Vector3d _pos, _pivot;
	float _pitch, _yaw, _roll;
	Vector3d _animPos;
	float _animPitch, _animYaw, _animRoll;
	bool _meshVisible, _hierVisible;
	bool _initialized;	
	
	void loadBinary(const char *&data, ModelNode *hierNodes, const Geoset *g);
	
	std::string toString();
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
	
	// Loaders
	void loadText();
	void loadBinary(const char *&data, uint length);
		
	// Savers
	
	std::string write3DOText();
	std::string writeOBJText();
};



#endif