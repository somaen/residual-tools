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

#ifndef _3dopatch_h
#define _3dopatch_h

#include <string>
#include <iostream>

// Multi-way converter for 3DO-files, currently very WIP.
// Borrows heavily from model.cpp in Residual

struct Vector2d {
	float _coords[2];
	void set(float x, float y);
	Vector2d(float x=0.0f, float y=0.0f);
	// TODO: Proper operator
	Vector2d operator*(float factor) {
		_coords[0] = _coords[0] * factor;
		_coords[1] = _coords[1] * factor;
		return *this;
	}
};

#endif