///////////////////////////////////////////////////////////////////////////
// FILE:                         maths.h                                 //
///////////////////////////////////////////////////////////////////////////
//                      BAHAMUT GRAPHICS LIBRARY                         //
//                        Author: Corbin Stark                           //
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Corbin Stark                                       //
//                                                                       //
// Permission is hereby granted, free of charge, to any person obtaining //
// a copy of this software and associated documentation files (the       //
// "Software"), to deal in the Software without restriction, including   //
// without limitation the rights to use, copy, modify, merge, publish,   //
// distribute, sublicense, and/or sell copies of the Software, and to    //
// permit persons to whom the Software is furnished to do so, subject to //
// the following conditions:                                             //
//                                                                       //
// The above copyright notice and this permission notice shall be        //
// included in all copies or substantial portions of the Software.       //
//                                                                       //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       //
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.//
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  //
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     //
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                //
///////////////////////////////////////////////////////////////////////////

#ifndef MATHS_H
#define MATHS_H

#include "defines.h"

GLOBAL const f32 PI = (f32)3.141592653589793238;

INTERNAL inline 
f32 deg_to_rad(f32 deg) {
	return deg * PI / 180.0f;
}

INTERNAL inline 
f32 rad_to_deg(f32 rad) {
	return rad * 180.0f / PI;
}

union vec2 {
	f32 e[2];
	struct {
		f32 x;
		f32 y;
	};
};

union vec3 {
	f32 e[3];
	struct {
		f32 x;
		f32 y;
		f32 z;
	};
	struct {
		vec2 xy;
		f32 _pad0;
	};
	struct {
		f32 _pad1;
		vec2 yz;
	};
};

union vec4 {
	f32 e[4];
	struct {
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	struct {
		vec3 xyz;
		f32 _pad0;
	};
	struct {
		vec2 xy;
		f32 _pad1;
		f32 _pad2;
	};
	struct {
		f32 _pad3;
		vec2 yz;
		f32 _pad4;
	};
	struct {
		f32 _pad5;
		f32 _pad6;
		vec2 zw;
	};
};

union Quaternion {
	f32 e[4];
	struct {
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	struct {
		f32 a;
		f32 k;
		f32 j;
		f32 i;
	};
	struct {
		vec3 xyz;
		f32 _pad0;
	};
	struct {
		vec2 xy;
		f32 _pad1;
		f32 _pad2;
	};
	struct {
		f32 _pad3;
		vec2 yz;
		f32 _pad4;
	};
	struct {
		f32 _pad5;
		f32 _pad6;
		vec2 zw;
	};
	vec4 xyzw;
};

INTERNAL inline 
vec2 V2(f32 x, f32 y) {
	vec2 vec;
	vec.x = x;
	vec.y = y;
	return vec;
}

INTERNAL inline 
vec3 V3(f32 x, f32 y, f32 z) {
	vec3 vec;
	vec.x = x;
	vec.y = y;
	vec.z = z;
	return vec;
}

INTERNAL inline 
vec3 V3(vec2 v2, f32 z) {
	vec3 vec;
	vec.x = v2.x;
	vec.y = v2.y;
	vec.z = z;
	return vec;
}

INTERNAL inline 
vec4 V4(f32 x, f32 y, f32 z, f32 w) {
	vec4 vec;
	vec.x = x;
	vec.y = y;
	vec.z = z;
	vec.w = w;
	return vec;
}

INTERNAL inline 
vec4 V4(vec3 v3, f32 w) {
	vec4 vec;
	vec.x = v3.x;
	vec.y = v3.y;
	vec.z = v3.z;
	vec.w = w;
	return vec;
}

INTERNAL inline 
vec4 V4(vec2 v21, vec2 v22) {
	vec4 vec;
	vec.x = v21.x;
	vec.y = v21.y;
	vec.z = v22.x;
	vec.w = v22.y;
	return vec;
}

INTERNAL inline 
f32 length(vec2 vec) {
#ifdef USE_SSE
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(SimdReals, SimdReals, 0x71)));
#else
	return sqrt(vec.x * vec.x + vec.y * vec.y);
#endif
}

INTERNAL inline 
void normalize(vec2* vec) {
	f32 len = length(*vec);
	if (len != 0) {
		vec->x /= len;
		vec->y /= len;
	}
}

INTERNAL inline 
vec2 normalize(vec2 vec) {
	f32 len = length(vec);
	if (len != 0) {
		return V2(vec.x / len, vec.y / len);
	}
	return vec;
}

INTERNAL inline
f32 dot(vec2 a, vec2 b) {
	return (a.x * b.x) + (a.y * b.y);
}

INTERNAL inline
f32 length(vec3 vec) {
#ifdef USE_SSE
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(SimdReals, SimdReals, 0x71)));
#else
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
}

INTERNAL inline
void normalize(vec3* vec) {
	f32 len = length(*vec);
	if (len != 0) {
		vec->x /= len;
		vec->y /= len;
		vec->z /= len;
	}
}

INTERNAL inline
vec3 normalize(vec3 vec) {
	f32 len = length(vec);
	if (len != 0) {
		return V3(vec.x / len, vec.y / len, vec.z / len);
	}
	return vec;
}

INTERNAL inline
f32 dot(vec3 a, vec3 b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

INTERNAL inline
vec3 cross(vec3 a, vec3 b) {
	return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

INTERNAL inline
f32 length(vec4 vec) {
#ifdef USE_SSE
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(SimdReals, SimdReals, 0x71)));
#else
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
#endif
}

INTERNAL inline
void normalize(vec4* vec) {
	f32 len = length(*vec);
	if (len != 0) {
		vec->x /= len;
		vec->y /= len;
		vec->z /= len;
		vec->w /= len;
	}
}

INTERNAL inline
vec4 normalize(vec4 vec) {
	f32 len = length(vec);
	if (len != 0) {
		return V4(vec.x / len, vec.y / len, vec.z / len, vec.w / len);
	}
	return vec;
}

INTERNAL inline
f32 dot(vec4 a, vec4 b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

//OPERATOR OVERLOADS
// VEC2

INTERNAL inline
vec2 operator+(vec2 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}

INTERNAL inline
vec2 operator-(vec2 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}

INTERNAL inline
vec2 operator*(vec2 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	return c;
}

INTERNAL inline
vec2 operator*(f32 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a * b.x;
	c.y = a * b.y;
	return c;
}

INTERNAL inline
vec2 operator*(vec2 a, f32 b) {
	vec2 c = { 0 };
	c.x = a.x * b;
	c.y = a.y * b;
	return c;
}

INTERNAL inline
vec2 operator/(vec2 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a.x / b.x;
	c.y = a.y / b.y;
	return c;
}

INTERNAL inline
vec2 operator/(f32 a, vec2 b) {
	vec2 c = { 0 };
	c.x = a / b.x;
	c.y = a / b.y;
	return c;
}

INTERNAL inline
vec2 operator/(vec2 a, f32 b) {
	vec2 c = { 0 };
	c.x = a.x / b;
	c.y = a.y / b;
	return c;
}

// VEC3 

INTERNAL inline
vec3 operator+(vec3 a, vec3 b) {
	vec3 c = { 0 };
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return c;
}

INTERNAL inline
vec3 operator-(vec3 a, vec3 b) {
	vec3 c = { 0 };
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}

INTERNAL inline
vec3 operator*(vec3 a, vec3 b) {
	vec3 c = { 0 };
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
	return c;
}

INTERNAL inline
vec3 operator*(f32 a, vec3 b) {
	vec3 c = { 0 };
	c.x = a * b.x;
	c.y = a * b.y;
	c.z = a * b.z;
	return c;
}

INTERNAL inline
vec3 operator/(vec3 a, vec3 b) {
	vec3 c = { 0 };
	c.x = a.x / b.x;
	c.y = a.y / b.y;
	c.z = a.z / b.z;
	return c;
}

// VEC4

INTERNAL inline
vec4 operator+(vec4 a, vec4 b) {
	vec4 c = { 0 };
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	c.w = a.w + b.w;
	return c;
}

INTERNAL inline
vec4 operator-(vec4 a, vec4 b) {
	vec4 c = { 0 };
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	c.w = a.w - b.w;
	return c;
}

INTERNAL inline
vec4 operator*(vec4 a, vec4 b) {
	vec4 c = { 0 };
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
	c.w = a.w * b.w;
	return c;
}

INTERNAL inline
vec4 operator*(f32 a, vec4 b) {
	vec4 c = { 0 };
	c.x = a * b.x;
	c.y = a * b.y;
	c.z = a * b.z;
	c.w = a * b.w;
	return c;
}

//NOTE: Not commutative.
INTERNAL inline
Quaternion operator*(Quaternion a, Quaternion b) {
	Quaternion c;
	c.w = ((a.w*b.w) - (a.x*b.x) - (a.y*b.y) - (a.z*b.z));
	c.x = ((a.w*b.x) + (a.x*b.w) + (a.y*b.z) - (a.z*b.y));
	c.y = ((a.w*b.y) - (a.x*b.z) + (a.y*b.w) + (a.z*b.x));
	c.z = ((a.w*b.z) + (a.x*b.y) - (a.y*b.x) + (a.z*b.w));
	return c;
}

INTERNAL inline
vec4 operator/(vec4 a, vec4 b) {
	vec4 c = { 0 };
	c.x = a.x / b.x;
	c.y = a.y / b.y;
	c.z = a.z / b.z;
	c.w = a.w / b.w;
	return c;
}

INTERNAL inline
bool operator==(vec2 a, vec2 b) {
	return (a.x == b.x && a.y == b.y);
}

INTERNAL inline
bool operator==(vec3 a, vec3 b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}

INTERNAL inline
bool operator==(vec4 a, vec4 b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

//================================================
//Description: 4x4 matrix for graphics calculations. 
//
//Comments: IMPORTANT! Matrices in OpenGL are
//	column-major. Accessing elements of this
//	matrix are done in the following way:
//------------------------------------------------
//		elements[column + row * 4]
//================================================
union mat4 {
	f32 elements[4 * 4];
	vec4 columns[4];
	struct {
		f32 m00, m01, m02, m03;
		f32 m10, m11, m12, m13;
		f32 m20, m21, m22, m23;
		f32 m30, m31, m32, m33;
	};
};

INTERNAL inline
mat4 identity() {
	mat4 mat =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return mat;
}

INTERNAL inline
mat4 operator*(const mat4 a, const mat4 b) {
	mat4 c = {};
	f32 data[4 * 4];
	for (u8 y = 0; y < 4; ++y) {
		for (u8 x = 0; x < 4; ++x) {
			f32 sum = 0.0f;
			for (u8 i = 0; i < 4; ++i) {
				sum += a.elements[x + i * 4] * b.elements[i + y * 4];
			}
			data[x + y * 4] = sum;
		}
	}
	memcpy(c.elements, data, 16 * sizeof(f32));
	return c;
}

INTERNAL inline
void operator*=(mat4& a, const mat4 b) {
	f32 data[4 * 4];
	for (u8 y = 0; y < 4; ++y) {
		for (u8 x = 0; x < 4; ++x) {
			f32 sum = 0.0f;
			for (u8 i = 0; i < 4; ++i) {
				sum += a.elements[x + i * 4] * b.elements[i + y * 4];
			}
			data[x + y * 4] = sum;
		}
	}
	memcpy(a.elements, data, 16 * sizeof(f32));
}

INTERNAL inline
mat4 translation(const f32 x, const f32 y, const f32 z) {
	mat4 mat = identity();
	mat.elements[0 + 3 * 4] = x;
	mat.elements[1 + 3 * 4] = y;
	mat.elements[2 + 3 * 4] = z;
	return mat;
}

INTERNAL inline
mat4 translation(const vec3 translation_vec) {
	return translation(translation_vec.x, translation_vec.y, translation_vec.z);
}

INTERNAL inline
mat4 scale(const f32 x, const f32 y, const f32 z) {
	mat4 mat = { 0 };
	mat.elements[0 + 0 * 4] = x;
	mat.elements[1 + 1 * 4] = y;
	mat.elements[2 + 2 * 4] = z;
	mat.elements[3 + 3 * 4] = 1.0f;
	return mat;
}

INTERNAL inline
mat4 scale(const vec3& scale_vec) {
	return scale(scale_vec.x, scale_vec.y, scale_vec.z);
}

INTERNAL inline
mat4 rotateX(f32 angle) {
	mat4 mat = identity();
	f32 theta = deg_to_rad(angle);
	f32 s = sin(theta);
	f32 c = cos(theta);

	mat.elements[1 + 1 * 4] = c;
	mat.elements[2 + 2 * 4] = c;
	mat.elements[2 + 1 * 4] = s;
	mat.elements[1 + 2 * 4] = -s;
	return mat;
}

INTERNAL inline
mat4 rotateY(f32 angle) {
	mat4 mat = identity();
	f32 theta = deg_to_rad(angle);
	f32 s = sin(theta);
	f32 c = cos(theta);

	mat.elements[0 + 0 * 4] = c;
	mat.elements[2 + 0 * 4] = -s;
	mat.elements[0 + 2 * 4] = s;
	mat.elements[2 + 2 * 4] = c;
	return mat;
}

INTERNAL inline
mat4 rotateZ(f32 angle) {
	mat4 mat = identity();
	f32 theta = deg_to_rad(angle);
	f32 s = sin(theta);
	f32 c = cos(theta);

	mat.elements[0 + 0 * 4] = c;
	mat.elements[1 + 0 * 4] = s;
	mat.elements[0 + 1 * 4] = -s;
	mat.elements[1 + 1 * 4] = c;
	return mat;
}

INTERNAL inline
mat4 rotation(f32 angle, f32 x, f32 y, f32 z) {
	mat4 mat = { 0 };
	f32 theta = deg_to_rad(angle);
	f32 s = sin(theta);
	f32 c = cos(theta);
	f32 cm1 = (1.0f - c);

	mat.elements[0 + 0 * 4] = x * x * cm1 + c;
	mat.elements[1 + 0 * 4] = y * x * cm1 + z * s;
	mat.elements[2 + 0 * 4] = x * z * cm1 - y * s;
	mat.elements[0 + 1 * 4] = x * y * cm1 - z * s;
	mat.elements[1 + 1 * 4] = y * y * cm1 + c;
	mat.elements[2 + 1 * 4] = y * z * cm1 + x * s;
	mat.elements[0 + 2 * 4] = x * z * cm1 + y * s;
	mat.elements[1 + 2 * 4] = y * z * cm1 - x * s;
	mat.elements[2 + 2 * 4] = z * z * cm1 + c;
	mat.elements[3 + 3 * 4] = 1.0f;
	return mat;
}

INTERNAL inline
mat4 rotation(f32 angle, const vec3 axis) {
	return rotation(angle, axis.x, axis.y, axis.z);
}

//2D Projection
INTERNAL inline
mat4 orthographic_projection(f32 x, f32 y, f32 width, f32 height, f32 near_plane, f32 far_plane) {
	mat4 mat = { 0 };
	mat.elements[0 + 0 * 4] = 2.0f / (width - x);
	mat.elements[1 + 1 * 4] = 2.0f / (y - height);
	mat.elements[2 + 2 * 4] = -2.0f / (far_plane - near_plane);
	mat.elements[0 + 3 * 4] = -((width + x) / (width - x));
	mat.elements[1 + 3 * 4] = -((y + height) / (y - height));
	mat.elements[2 + 3 * 4] = -((far_plane + near_plane) / (far_plane - near_plane));
	mat.elements[3 + 3 * 4] = 1.0f;
	return mat;
}

//3D Projection
INTERNAL inline
mat4 perspective_projection(f32 fov, f32 aspect_ratio, f32 near_plane, f32 far_plane) {
	mat4 mat = { 0 };
	float radFov = deg_to_rad(fov);
	mat.elements[0 + 0 * 4] = (1 / tan(radFov * 0.5f)) / aspect_ratio;
	mat.elements[1 + 1 * 4] = 1 / tan(radFov * 0.5f);
	mat.elements[2 + 2 * 4] = -((far_plane + near_plane) / (far_plane - near_plane));
	mat.elements[2 + 3 * 4] = -((2.0f * near_plane * far_plane) / (far_plane - near_plane));
	mat.elements[3 + 2 * 4] = -1.0f;
	return mat;
}

//Might have to swap the order of these
INTERNAL inline
mat4 quaternion_matrix(Quaternion q) {
	mat4 mat = identity();
	//q^2 + x^2 - y^2 - z^2
	mat.elements[0 + 0 * 4] = (q.w*q.w) + (q.x*q.x) - (q.y*q.y) - (q.z*q.z);
	//2xy - 2wz
	mat.elements[0 + 1 * 4] = (2 * (q.x*q.y)) - (2 * (q.w*q.z));
	//2xz + 2wy
	mat.elements[0 + 2 * 4] = (2 * (q.x*q.z)) + (2 * (q.w*q.y));
	//2xy + 2wz
	mat.elements[1 + 0 * 4] = (2 * (q.x*q.y)) + (2 * (q.w*q.z));
}

INTERNAL inline
mat4 create_transformation_matrix(f32 x, f32 y, f32 z, f32 rotX, f32 rotY, f32 rotZ, f32 scaleX, f32 scaleY, f32 scaleZ) {
	mat4 mat = identity();
	mat *= translation(x, y, z);
	mat *= rotation(rotX, 1, 0, 0);
	mat *= rotation(rotY, 0, 1, 0);
	mat *= rotation(rotZ, 0, 0, 1);
	mat *= scale(scaleX, scaleY, scaleZ);
	return mat;
}

INTERNAL inline
mat4 create_transformation_matrix(const vec3 translation, const vec3 rotation, const vec3 scale_vec) {
	return create_transformation_matrix(
		translation.x, translation.y, translation.z,
		rotation.x, rotation.y, rotation.z,
		scale_vec.x, scale_vec.y, scale_vec.z
	);
}

//ALGORITHMS

INTERNAL inline
mat4 inverse(const mat4 mat) {
	mat4 result;

	result.elements[0] =
		(mat.elements[5] * mat.elements[10] * mat.elements[15]) -
		(mat.elements[5] * mat.elements[11] * mat.elements[14]) -
		(mat.elements[9] * mat.elements[6] * mat.elements[15]) +
		(mat.elements[9] * mat.elements[7] * mat.elements[14]) +
		(mat.elements[13] * mat.elements[6] * mat.elements[11]) -
		(mat.elements[13] * mat.elements[7] * mat.elements[10]);

	result.elements[1] =
		(-mat.elements[1] * mat.elements[10] * mat.elements[15]) +
		(mat.elements[1] * mat.elements[11] * mat.elements[14]) +
		(mat.elements[9] * mat.elements[2] * mat.elements[15]) -
		(mat.elements[9] * mat.elements[3] * mat.elements[14]) -
		(mat.elements[13] * mat.elements[2] * mat.elements[11]) +
		(mat.elements[13] * mat.elements[3] * mat.elements[10]);

	result.elements[2] =
		(mat.elements[1] * mat.elements[6] * mat.elements[15]) -
		(mat.elements[1] * mat.elements[7] * mat.elements[14]) -
		(mat.elements[5] * mat.elements[2] * mat.elements[15]) +
		(mat.elements[5] * mat.elements[3] * mat.elements[14]) +
		(mat.elements[13] * mat.elements[2] * mat.elements[7]) -
		(mat.elements[13] * mat.elements[3] * mat.elements[6]);

	result.elements[3] =
		(-mat.elements[1] * mat.elements[6] * mat.elements[11]) +
		(mat.elements[1] * mat.elements[7] * mat.elements[10]) +
		(mat.elements[5] * mat.elements[2] * mat.elements[11]) -
		(mat.elements[5] * mat.elements[3] * mat.elements[10]) -
		(mat.elements[9] * mat.elements[2] * mat.elements[7]) +
		(mat.elements[9] * mat.elements[3] * mat.elements[6]);

	result.elements[4] =
		(-mat.elements[4] * mat.elements[10] * mat.elements[15]) +
		(mat.elements[4] * mat.elements[11] * mat.elements[14]) +
		(mat.elements[8] * mat.elements[6] * mat.elements[15]) -
		(mat.elements[8] * mat.elements[7] * mat.elements[14]) -
		(mat.elements[12] * mat.elements[6] * mat.elements[11]) +
		(mat.elements[12] * mat.elements[7] * mat.elements[10]);

	result.elements[5] =
		(mat.elements[0] * mat.elements[10] * mat.elements[15]) -
		(mat.elements[0] * mat.elements[11] * mat.elements[14]) -
		(mat.elements[8] * mat.elements[2] * mat.elements[15]) +
		(mat.elements[8] * mat.elements[3] * mat.elements[14]) +
		(mat.elements[12] * mat.elements[2] * mat.elements[11]) -
		(mat.elements[12] * mat.elements[3] * mat.elements[10]);

	result.elements[6] =
		(-mat.elements[0] * mat.elements[6] * mat.elements[15]) +
		(mat.elements[0] * mat.elements[7] * mat.elements[14]) +
		(mat.elements[4] * mat.elements[2] * mat.elements[15]) -
		(mat.elements[4] * mat.elements[3] * mat.elements[14]) -
		(mat.elements[12] * mat.elements[2] * mat.elements[7]) +
		(mat.elements[12] * mat.elements[3] * mat.elements[6]);

	result.elements[7] =
		(mat.elements[0] * mat.elements[6] * mat.elements[11]) -
		(mat.elements[0] * mat.elements[7] * mat.elements[10]) -
		(mat.elements[4] * mat.elements[2] * mat.elements[11]) +
		(mat.elements[4] * mat.elements[3] * mat.elements[10]) +
		(mat.elements[8] * mat.elements[2] * mat.elements[7]) -
		(mat.elements[8] * mat.elements[3] * mat.elements[6]);

	result.elements[8] =
		(mat.elements[4] * mat.elements[9] * mat.elements[15]) -
		(mat.elements[4] * mat.elements[11] * mat.elements[13]) -
		(mat.elements[8] * mat.elements[5] * mat.elements[15]) +
		(mat.elements[8] * mat.elements[7] * mat.elements[13]) +
		(mat.elements[12] * mat.elements[5] * mat.elements[11]) -
		(mat.elements[12] * mat.elements[7] * mat.elements[9]);

	result.elements[9] =
		(-mat.elements[0] * mat.elements[9] * mat.elements[15]) +
		(mat.elements[0] * mat.elements[11] * mat.elements[13]) +
		(mat.elements[8] * mat.elements[1] * mat.elements[15]) -
		(mat.elements[8] * mat.elements[3] * mat.elements[13]) -
		(mat.elements[12] * mat.elements[1] * mat.elements[11]) +
		(mat.elements[12] * mat.elements[3] * mat.elements[9]);

	result.elements[10] =
		(mat.elements[0] * mat.elements[5] * mat.elements[15]) -
		(mat.elements[0] * mat.elements[7] * mat.elements[13]) -
		(mat.elements[4] * mat.elements[1] * mat.elements[15]) +
		(mat.elements[4] * mat.elements[3] * mat.elements[13]) +
		(mat.elements[12] * mat.elements[1] * mat.elements[7]) -
		(mat.elements[12] * mat.elements[3] * mat.elements[5]);

	result.elements[11] =
		(-mat.elements[0] * mat.elements[5] * mat.elements[11]) +
		(mat.elements[0] * mat.elements[7] * mat.elements[9]) +
		(mat.elements[4] * mat.elements[1] * mat.elements[11]) -
		(mat.elements[4] * mat.elements[3] * mat.elements[9]) -
		(mat.elements[8] * mat.elements[1] * mat.elements[7]) +
		(mat.elements[8] * mat.elements[3] * mat.elements[5]);

	result.elements[12] =
		(-mat.elements[4] * mat.elements[9] * mat.elements[14]) +
		(mat.elements[4] * mat.elements[10] * mat.elements[13]) +
		(mat.elements[8] * mat.elements[5] * mat.elements[14]) -
		(mat.elements[8] * mat.elements[6] * mat.elements[13]) -
		(mat.elements[12] * mat.elements[5] * mat.elements[10]) +
		(mat.elements[12] * mat.elements[6] * mat.elements[9]);

	result.elements[13] =
		(mat.elements[0] * mat.elements[9] * mat.elements[14]) -
		(mat.elements[0] * mat.elements[10] * mat.elements[13]) -
		(mat.elements[8] * mat.elements[1] * mat.elements[14]) +
		(mat.elements[8] * mat.elements[2] * mat.elements[13]) +
		(mat.elements[12] * mat.elements[1] * mat.elements[10]) -
		(mat.elements[12] * mat.elements[2] * mat.elements[9]);

	result.elements[14] =
		(-mat.elements[0] * mat.elements[5] * mat.elements[14]) +
		(mat.elements[0] * mat.elements[6] * mat.elements[13]) +
		(mat.elements[4] * mat.elements[1] * mat.elements[14]) -
		(mat.elements[4] * mat.elements[2] * mat.elements[13]) -
		(mat.elements[12] * mat.elements[1] * mat.elements[6]) +
		(mat.elements[12] * mat.elements[2] * mat.elements[5]);

	result.elements[15] =
		(mat.elements[0] * mat.elements[5] * mat.elements[10]) -
		(mat.elements[0] * mat.elements[6] * mat.elements[9]) -
		(mat.elements[4] * mat.elements[1] * mat.elements[10]) +
		(mat.elements[4] * mat.elements[2] * mat.elements[9]) +
		(mat.elements[8] * mat.elements[1] * mat.elements[6]) -
		(mat.elements[8] * mat.elements[2] * mat.elements[5]);

	f32 determinate =
		mat.elements[0] * result.elements[0] +
		mat.elements[1] * result.elements[4] +
		mat.elements[2] * result.elements[8] +
		mat.elements[3] * result.elements[12];

	if (determinate == 0) {
		BMT_LOG(WARNING, "Matrix could not be inverted! Determinate = 0");
		return result;
	}

	determinate = 1.0 / determinate;

	for (u16 i = 0; i < 16; ++i) {
		result.elements[i] *= determinate;
	}

	return result;
}

INTERNAL inline
bool point_inside_triangle(vec3 point, vec3 tri1, vec3 tri2, vec3 tri3) {
	vec3 u = tri2 - tri1;
	vec3 v = tri3 - tri1;
	vec3 w = point - tri1;
	vec3 n = cross(u, v);

	f32 y = (dot(cross(u, w), n) / dot(n, n));
	f32 b = (dot(cross(u, w), n) / dot(n, n));
	f32 a = 1 - y - b;
	vec3 p = (a * tri1) + (b * tri2) + (y * tri3);

	return (a >= 0 && a <= 1 && b >= 0 && b <= 1 && y >= 0 && y <= 1);
}

INTERNAL inline
mat4 look_at(const vec3 camera, const vec3 center, const vec3 up = V3(0, 1, 0)) {
	mat4 mat = identity();

	vec3 dir = normalize(center - camera);
	vec3 upNorm = normalize(up);
	vec3 right = normalize(cross(dir, upNorm));
	upNorm = cross(right, dir);

	mat.m00 = right.x;
	mat.m10 = right.y;
	mat.m20 = right.z;
	mat.m01 = upNorm.x;
	mat.m11 = upNorm.y;
	mat.m21 = upNorm.z;
	mat.m02 = -dir.x;
	mat.m12 = -dir.y;
	mat.m22 = -dir.z;
	mat.m30 = -dot(right, camera);
	mat.m31 = -dot(upNorm, camera);
	mat.m32 = dot(dir, camera);

	return mat;
}

struct Camera {
	f32 x;
	f32 y;
	f32 z;
	f32 pitch;
	f32 yaw;
	f32 roll;
};

INTERNAL inline
mat4 create_view_matrix(Camera cam) {
	mat4 mat = identity();
	mat *= rotation(cam.pitch, 1, 0, 0);
	mat *= rotation(cam.yaw, 0, 1, 0);
	mat *= rotation(cam.roll, 0, 0, 1);
	mat *= translation(-cam.x, -cam.y, -cam.z);
	return mat;
}

INTERNAL inline
void move_cam_forward(Camera* cam, f32 units) {
	cam->z -= cos(deg_to_rad(cam->yaw))   * units;
	cam->x += sin(deg_to_rad(cam->yaw))   * units;
	cam->y -= sin(deg_to_rad(cam->pitch)) * units;
}

INTERNAL inline
void move_cam_backwards(Camera* cam, f32 units) {
	cam->z += cos(deg_to_rad(cam->yaw))   * units;
	cam->x -= sin(deg_to_rad(cam->yaw))   * units;
	cam->y += sin(deg_to_rad(cam->pitch)) * units;
}

INTERNAL inline
void move_cam_right(Camera* cam, f32 units) {
	cam->x += cos(deg_to_rad(cam->yaw)) * units;
	cam->z += sin(deg_to_rad(cam->yaw)) * units;
}

INTERNAL inline
void move_cam_left(Camera* cam, f32 units) {
	cam->x -= cos(deg_to_rad(cam->yaw)) * units;
	cam->z -= sin(deg_to_rad(cam->yaw)) * units;
}

#endif