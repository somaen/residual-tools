#ifndef FILETOOLS_H
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


#define FILETOOLS_H

#include <fstream>
#include <string>
#include <sstream>

template<typename T>
struct Vector3 {
	T _x;
	T _y;
	T _z;
	void setVal(T x, T y, T z) {
		_x = x;
		_y = y;
		_z = z;
	}
};

struct Vector2d {
	float x;
	float y;
	std::string toString() {
		std::stringstream ss;
		ss << x << " " << y;
		return ss.str();
	}
};

struct Vector3d {
	float _x;
	float _y;
	float _z;

	Vector3d() : _x(0), _y(0), _z(0) {}
	Vector3d(float x, float y, float z) : _x(x), _y(y), _z(z) {}
	float dot(Vector3d vec) {
		float retVal = 0.0f;
		retVal += _x * vec._x;
		retVal += _y * vec._y;
		retVal += _z * vec._z;
		return retVal;
	}

	std::string toString() {
		std::stringstream ss;
		ss << _x << " " << _y << " " << _z;
		return ss.str();
	}

	Vector3d operator *(float val) {
		Vector3d vec(*this);
		vec._x *= val;
		vec._y *= val;
		vec._z *= val;
		return vec;
	}
	Vector3d operator+= (Vector3d vec) {
		_x += vec._x;
		_y += vec._y;
		_z += vec._z;
	}
};

struct Vector4d {
	float _x;
	float _y;
	float _z;
	float _w;
	std::string toString() {
		std::stringstream ss;
		ss << _x << " " << _y << " " << _z << " " << _w;
		return ss.str();
	}
};

float readFloat(std::istream& file) {
	float retVal = 0.0f;
	file.read((char*)&retVal, 4);
	return retVal;
}

int readInt(std::istream& file) {
	int retVal = 0;
	file.read((char*)&retVal, 4);
	return retVal;
}

short readShort(std::istream& file) {
	short retVal = 0;
	file.read((char*)&retVal, 2);
	return retVal;
}

int readByte(std::istream& file) {
	char retVal = 0;
	file.read((char*)&retVal, 1);
	return retVal;
}

std::string readString(std::istream& file) {
	int strLength = 0;
	file.read((char*)&strLength, 4);
	char* readString = new char[strLength];
	file.read(readString, strLength);

	std::string retVal(readString);
	delete readString;

	return retVal;
}

std::string readCString(std::istream &file, int len) {
	char *str = new char[len];
	file.read(str, len);
	std::string retVal = std::string(str);
	delete[] str;
	return retVal;
}

Vector2d *readVector2d(std::istream& file, int count = 1) {
	Vector2d *vec2d = new Vector2d[count];
	for (int i = 0; i < count; i++) {
		vec2d[i].x = readFloat(file);
		vec2d[i].y = readFloat(file);
	}
	return vec2d;
}

Vector3d *readVector3d(std::istream& file, int count = 1) {
	Vector3d *vec3d = new Vector3d[count];
	for (int i = 0; i < count; i++) {
		vec3d[i]._x = readFloat(file);
		vec3d[i]._y = readFloat(file);
		vec3d[i]._z = readFloat(file);
	}
	return vec3d;
}

Vector4d *readVector4d(std::istream& file) {
	Vector4d *vec4d = new Vector4d();
	vec4d->_x = readFloat(file);
	vec4d->_y = readFloat(file);
	vec4d->_z = readFloat(file);
	vec4d->_w = readFloat(file);
	return vec4d;
}

#endif
