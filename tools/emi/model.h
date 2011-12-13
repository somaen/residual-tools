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

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <stdint.h>
#include <map>
#include <vector>
#include "matrix.h"

struct Colormap {
	unsigned char r, g, b, a;
};

class Mesh;
class Lab;

class Material {
	uint32_t *_texIDs;
	uint32_t _numTextures;
	Lab *_lab;
	void loadTGATexture(std::string filename);
	void loadSURTexture(std::string filename);
public:
	Material() { _lab = 0; }
	void setLab(Lab *lab) { _lab = lab; }
	void loadTexture(std::string filename);
	void bindTexture(int index = 0);
};

class MeshFace {
	Vector3<int> *_indexes;
	uint32_t _faceLength;
	uint32_t _numFaces;
	uint32_t _hasTexture;
	uint32_t _texID;
	uint32_t _flags;
	Mesh *_parent;
public:
	void print() {
		std::cout << "FACELENGTH: " << _faceLength << "\n\n\n\n";
		for (int i=0; i < _faceLength; i++) {
			std::cout << i << ": ";
			std::cout << _indexes[i]._x << " ";
			std::cout << _indexes[i]._y << " ";
			std::cout << _indexes[i]._z << " " << std::endl;
		}
		std::cout << "endoffaceendoffaceendoffaceendoffaceendofface\n";
	}
	MeshFace() : _numFaces(0), _hasTexture(0), _texID(0), _flags(0) { }
	void loadFace(std::istream *file);
	void setParent(Mesh *m) { _parent = m; }
	void render();
};

class Keyframe {
public:
	float _time;
	Vector3d *_vec3d;
	Vector4d *_vec4d;
};

class KeyframeList {
public:
	Keyframe *_frames;
	int _numFrames;
	float _time;
	int _operation;
	KeyframeList(int num, int op) : _numFrames(num), _operation(op) {
		_frames = new Keyframe[_numFrames];
	}
};

class Bone {
	std::string _name;
	Bone *_parent;
	Bone *_child;
	Bone *_sibling;
	Vector3d *_pos;
	Vector3d *_rot;
	float _angle;
	std::vector<int> _verts;
	std::vector<float> _wgts;
	KeyframeList *_keyframes;
public:
	Bone() : _parent(0), _child(0), _sibling(0), _angle(0.0f), _keyframes(0) {}
	void addParent(Bone *node);
	void addChild(Bone *node);
	void addSibling(Bone *node);
	bool hasSibling(Bone *node);
	bool hasChild(Bone *node);
	void setName(std::string name) { _name = name; }
	void setPos(Vector3d *pos) { _pos = pos; }
	void setRot(Vector3d *rot) { _rot = rot; }
	void setAngle(float angle) { _angle = angle; }
	void addVertex(int num, float wgt);
	void setKeyframes(KeyframeList* keyframes) { _keyframes = keyframes; }

	std::string getName() { return _name; }

	void playAnim(Vector3d *origVerts, Vector3d *verts, int num, float time) {
		int foundFrame = -1;
		for (int i = 0; i < _keyframes->_numFrames; i++) {
			if (time > _keyframes->_frames[i]._time) {
				foundFrame = i;
				break;
			}
		}
		if (foundFrame == -1)
			foundFrame = 0;

		Keyframe *key = _keyframes->_frames + foundFrame;

		int vert = 0;
		float weight = 0;

		if (_keyframes->_operation == 3) {
			std::cout << "Operation 3 not supported yet";
		} else if (_keyframes->_operation == 4) {
			Matrix3 mat(*(key->_vec4d));
			//std::cout << "Performing operation 4 on " << _name << std::endl;
			std::cout << "Bone " << _name << " has " << _verts.size() << " Vertices\n";
			for (int i = 0; i < _verts.size(); i++) {
				vert = _verts[i];
				weight = _wgts[i];

				verts[vert] += mat.multVec(origVerts[vert]) * weight;
			}
		} else {
			std::cout << "Unknown operation: " << _keyframes->_operation << std::endl;
		}
	}
};

class Animation {
public:
	std::string _name;
	float _timelen;
	int _numBones;
	int _numKeyFrames;
	std::vector<Bone*> _bones;
	void play(Vector3d *origVerts, Vector3d *verts, int num, int time) {
		std::cout << "Playing animation with " << _bones.size() << " bones" << std::endl;
		std::vector<Bone*>::iterator it;
		Bone* bone;
		for (it = _bones.begin(); it != _bones.end(); it++) {
			bone = *it;
			bone->playAnim(origVerts, verts, num, time);
		}
	}
};

class Mesh {
	int _numVertices;
	Vector3d *_vertices;
	Vector3d *_animVerts;
	Vector3d *_normals;
	Colormap *_colorMap;
	Vector2d *_texVerts;

	uint32_t _numFaces;
	MeshFace *_faces;
	uint32_t _numTextures;
	std::string *_texNames;
	Material *_mats;

	Bone *_bones;
	int _numBones;
	std::map<std::string, Bone*> _boneMap;

	Animation *_anim;

	Lab *_lab;

	// Stuff I dont know how to use:
	Vector4d *_sphereData;
	Vector3d *_boxData;
	Vector3d *_boxData2;
	int _numTexSets;
	int _setType;

public:
	Mesh() : _lab(0), _animVerts(0) { }
	void setLab(Lab *lab) { _lab = lab; }
	void setTex(int index) { _mats[index].bindTexture(); }
	void loadMesh(std::string fileName);
	void loadSkeleton(std::string filename);
	void loadAnimation(std::string filename);
	void prepareForRender();
	void prepare();
	void render();
};

#endif
