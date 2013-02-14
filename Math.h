#ifndef MATFUNS_H
#define MATFUNS_H
#include <GL/gl.h>

typedef const GLfloat CGLfloat;

// Вектор из 3 элементов.
typedef union{
	struct{
		GLfloat x,y,z;
	};
	GLfloat v[3];
} Vector3f;

typedef const Vector3f CVector3f;

// Вектор из 4 элементов.
typedef union{
	struct{
		GLfloat x,y,z,w;
	};
	GLfloat v[4];
} Vector4f;

typedef const Vector4f CVector4f;

// Матрица 4x4.
typedef union{
	struct{
		GLfloat 
		_11, _12, _13, _14,
		_21, _22, _23, _24,
		_31, _32, _33, _34,
		_41, _42, _43, _44;
	};
	GLfloat m[16];
} Mat4x4;

typedef const Mat4x4 CMat4x4;

/*------------ Функции для работы с векторами ------------*/

// Вектора из 3 элементов (x,y,z).
GLfloat Vec3Length(CVector3f* v);
void  	Vec3Normalize(Vector3f* v);
void  	Vec3Sub(Vector3f* res, CVector3f* v1, CVector3f* v2);// v1 - v2
void  	Vec3Add(Vector3f* res, CVector3f* v1, CVector3f* v2);// v1 + v2
GLfloat Vec3DotProduct(CVector3f* v1, CVector3f* v2);
void  	Vec3CrossProduct(Vector3f* res, CVector3f* v1, CVector3f* v2);

/*------------ Функции для работы с матрицами ------------*/

// Матрица для перевода точки из глобальной(правой) сис.координат
// в систему координат(правую) камеры. Наблядатель смотрит в направлени
// положительной оси z.
void Mat4x4View(Mat4x4* m, CVector3f* Eye, CVector3f* Target, CVector3f* Up);

// Матрица для перевода точки из системы координат камеры в 
// однородное пространство clip space(левое пространтсво).
// Перспективная проекция.
void Mat4x4Pers(Mat4x4* m, GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f);

// Матрица для перевода точки из системы координат камеры в 
// однородное пространство clip space(левое пространтсво).
// Ортогональная проекция.
void Mat4x4Ortho(Mat4x4* m, GLfloat width, GLfloat height, GLfloat n, GLfloat f);

// Умножение матрицы на вектор.
void Mat4x4Vec4Mult(Vector4f* res, CMat4x4* m, CVector4f* v);

// Умножение матрицы на матрицу.
void Mat4x4Mult(Mat4x4* res, CMat4x4* m1, CMat4x4* m2);

// Матрица переноса.
void Mat4x4Translate(Mat4x4* m, GLfloat x, GLfloat y, GLfloat z);

// Матрица масштабирования.
void Mat4x4Scale(Mat4x4* m, GLfloat x, GLfloat y, GLfloat z);

// Матрица поворота.
// Аргумент axis принимает значение 'x','y' или 'z'.
// Угол задан в радианах.
void Mat4x4Rotate(Mat4x4* m, char axis, GLfloat angle);

// Единичная матрица.
void Mat4x4Identity(Mat4x4* m);

#endif // MATFUNS_H
