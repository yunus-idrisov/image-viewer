#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H
#include "GlobalTypes.h"
#include "Math.h"

WindowInfo winInfo = { 0, 0, 0, 800, 600, 800.0f/600.0f, GL_TRUE, GL_FALSE };

Mat4x4 worldMat, viewMat, orthoMat, PVW;
Vector3f target = {0,0,0}, eye = {1,0,0}, up = {0,1,0};

// Переменные используемые для анимации перелистывания.
GLfloat   zOffset = 0.0f, yOffset = 0.0f;
GLboolean isAnimationEnable = GL_TRUE;

GLuint vao;
GLuint verBuffer;

GLuint shader;
// Ссылки на переменные из шейдера.
GLuint winWidthID;
GLuint winHeightID;
GLuint imageWidthID;  
GLuint imageHeihgtID;
GLuint imageSmp;
GLuint zOffsetID;
GLuint yOffsetID;
GLuint PVWID;

// Отображаемая текстура.
TextureInfo gTexInfo = {0,0,0,0};
// Коэффициент масштабирования( [0.1, 1.0] ).
GLfloat imageScale = 1.0f;

/*
 * Режимы работы программы:
 * 1 - самый быстрый,
 * 2 - средний,
 * 3 - самый медленный.
 * Чем выше номер режима, тем меньше памяти
 * необходимо программе.
 */
int mode = 3;

#endif // GLOBAL_VARS_H
