#include "Math.h"
#include <math.h>
#include <string.h>
#include <ctype.h>

GLfloat Vec3Length(CVector3f* v){
	return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

void  Vec3Normalize(Vector3f* v){
	GLfloat len = Vec3Length(v);
	if( len != 0.0f ){
		v->x = v->x/len;
		v->y = v->y/len;
		v->z = v->z/len;
	}
}

void  Vec3Subt(Vector3f* res, CVector3f* v1, CVector3f* v2){
	res->x = v1->x - v2->x;
	res->y = v1->y - v2->y;
	res->z = v1->z - v2->z;
}

void  Vec3Addn(Vector3f* res, CVector3f* v1, CVector3f* v2){
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
	res->z = v1->z + v2->z;
}

GLfloat Vec3DotProduct(CVector3f* v1, CVector3f* v2){
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

void Vec3CrossProduct(Vector3f* res, CVector3f* v1, CVector3f* v2){
	res->x = v1->y*v2->z - v1->z*v2->y;
	res->y = -(v1->x*v2->z - v1->z*v2->x);
	res->z = v1->x*v2->y - v1->y*v2->x;
}

// Matrices.

void Mat4x4View(Mat4x4* m, CVector3f* Eye, CVector3f* Target, CVector3f* Up){
	Vector3f w;
	Vec3Subt(&w, Target, Eye);
	Vec3Normalize(&w);

	Vector3f u;
	Vec3CrossProduct(&u, Up, &w);
	Vec3Normalize(&u);

	Vector3f v;
	Vec3CrossProduct(&v, &w, &u);
	Vec3Normalize(&v);
	m->_11 = u.x; m->_12 = u.y; m->_13 = u.z; m->_14 = -Vec3DotProduct(&u,Eye);
	m->_21 = v.x; m->_22 = v.y; m->_23 = v.z; m->_24 = -Vec3DotProduct(&v,Eye);
	m->_31 = w.x; m->_32 = w.y; m->_33 = w.z; m->_34 = -Vec3DotProduct(&w,Eye);
	m->_41 =  0 ; m->_42 =  0 ; m->_43 =  0 ; m->_44 = 1.0f;
}

void Mat4x4Pers(Mat4x4* m, GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f){
	GLfloat r = aspect;
	GLfloat d = 1.0f/tanf(fov/2.0f);
	memset(m, 0, sizeof(GLfloat)*16);
	m->_11 = -1.0f/r*d;// -1 нужна т.к.система координат(a именно, homogeneous clip space),
					   // в которой находится точка после выхода из vertex shader, 
					   // является левой.
	m->_22 = d;
	m->_33 = -(n + f)/(n - f);
	m->_34 = 2.0f*n*f/(n - f);
	m->_43 = 1;
}

void Mat4x4Ortho(Mat4x4* m, GLfloat width, GLfloat height, GLfloat n, GLfloat f){
	memset(m, 0, sizeof(GLfloat)*16);
	m->_11 = -2/width;
	m->_22 =  2/height;
	m->_33 = 2/(f - n);
	m->_34 = -(f + n)/(f - n);
	m->_44 = 1;
}

void Mat4x4Vec4Mult(Vector4f* res, CMat4x4* m, CVector4f* v){
	Vector4f t;
	t.x = m->_11*v->x + m->_12*v->y + m->_13*v->z + m->_14*v->w;
	t.y = m->_21*v->x + m->_22*v->y + m->_23*v->z + m->_24*v->w;
	t.z = m->_31*v->x + m->_32*v->y + m->_33*v->z + m->_34*v->w;
	t.w = m->_41*v->x + m->_42*v->y + m->_43*v->z + m->_44*v->w;
	res->x = t.x;
	res->y = t.y;
	res->z = t.z;
	res->w = t.w;
}

void Mat4x4Mult(Mat4x4* res, CMat4x4* m1, CMat4x4* m2){
	Vector4f v;
	v.x = m1->_11*m2->_11 + m1->_12*m2->_21 + m1->_13*m2->_31 + m1->_14*m2->_41;
	v.y = m1->_11*m2->_12 + m1->_12*m2->_22 + m1->_13*m2->_32 + m1->_14*m2->_42;
	v.z = m1->_11*m2->_13 + m1->_12*m2->_23 + m1->_13*m2->_33 + m1->_14*m2->_43;
	v.w = m1->_11*m2->_14 + m1->_12*m2->_24 + m1->_13*m2->_34 + m1->_14*m2->_44;
	res->_11 = v.x; res->_12 = v.y; res->_13 = v.z; res->_14 = v.w;

	v.x = m1->_21*m2->_11 + m1->_22*m2->_21 + m1->_23*m2->_31 + m1->_24*m2->_41;
	v.y = m1->_21*m2->_12 + m1->_22*m2->_22 + m1->_23*m2->_32 + m1->_24*m2->_42;
	v.z = m1->_21*m2->_13 + m1->_22*m2->_23 + m1->_23*m2->_33 + m1->_24*m2->_43;
	v.w = m1->_21*m2->_14 + m1->_22*m2->_24 + m1->_23*m2->_34 + m1->_24*m2->_44;
	res->_21 = v.x; res->_22 = v.y; res->_23 = v.z; res->_24 = v.w;

	v.x = m1->_31*m2->_11 + m1->_32*m2->_21 + m1->_33*m2->_31 + m1->_34*m2->_41;
	v.y = m1->_31*m2->_12 + m1->_32*m2->_22 + m1->_33*m2->_32 + m1->_34*m2->_42;
	v.z = m1->_31*m2->_13 + m1->_32*m2->_23 + m1->_33*m2->_33 + m1->_34*m2->_43;
	v.w = m1->_31*m2->_14 + m1->_32*m2->_24 + m1->_33*m2->_34 + m1->_34*m2->_44;
	res->_31 = v.x; res->_32 = v.y; res->_33 = v.z; res->_34 = v.w;

	v.x = m1->_41*m2->_11 + m1->_42*m2->_21 + m1->_43*m2->_31 + m1->_44*m2->_41;
	v.y = m1->_41*m2->_12 + m1->_42*m2->_22 + m1->_43*m2->_32 + m1->_44*m2->_42;
	v.z = m1->_41*m2->_13 + m1->_42*m2->_23 + m1->_43*m2->_33 + m1->_44*m2->_43;
	v.w = m1->_41*m2->_14 + m1->_42*m2->_24 + m1->_43*m2->_34 + m1->_44*m2->_44;
	res->_41 = v.x; res->_42 = v.y; res->_43 = v.z; res->_44 = v.w;
}

void Mat4x4Translate(Mat4x4* m, GLfloat x, GLfloat y, GLfloat z){
	memset(m, 0, sizeof(GLfloat)*16);
	m->_11 = m->_22 = m->_33 = m->_44 = 1;
	m->_14 = x;
	m->_24 = y;
	m->_34 = z;
}

void Mat4x4Scale(Mat4x4* m, GLfloat x, GLfloat y, GLfloat z){
	memset(m, 0, sizeof(GLfloat)*16);
	m->_11 = x;
	m->_22 = y;
	m->_33 = z;
	m->_44 = 1.0f;
}

void Mat4x4Rotate(Mat4x4* m, char axis, GLfloat angle){
	if( !isalpha(axis) )
		return;
	else axis = tolower(axis);
	memset(m, 0, sizeof(GLfloat)*16);
	GLfloat s = sinf(angle);
	GLfloat c = cosf(angle);
	switch( axis ){
		case 'x' : m->_11 = 1.0f; m->_22 = c; m->_23 = -s; m->_32 = s; m->_33 = c; m->_44 = 1.0f; break;
		case 'y' : m->_11 = c; m->_13 = s; m->_22 = 1.0f; m->_31 = -s; m->_33 = c; m->_44 = 1.0f; break;
		case 'z' : m->_11 = c; m->_12 = -s; m->_21 = s; m->_22 = c; m->_22 = 1.0f; m->_44 = 1.0f; break;
	};
}

void Mat4x4Identity(Mat4x4* m){
	memset(m, 0, sizeof(GLfloat)*16);
	m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
}
