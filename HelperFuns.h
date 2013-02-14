#ifndef HELPERFUNS_H
#define HELPERFUNS_H
#include <GL/gl.h>

// Функция CreateProgram создаёт шейдерную программу из 
// вершинного и фрагментного шейдеров.
// Если произошла ошибка возвращается 0.
GLuint CreateProgram(const char *vertex_shader_path, const char *fragment_shader_path);

typedef struct{
	GLuint textureID;
	GLuint width;
	GLuint height;
} TextureInfo;

/*
 Создаёт текстуру OpenGL и возвращает её идентификатор,
 ширину и высоту.
 Имя текстуры объязательно должно содержать расширение.
 Если произошла ошибка, то в поле textureID 
 структуры TextureInfo записывается 0.
*/
TextureInfo loadTexture(const char *texturePath);

// Поддерживаемые форматы изображений.
enum SUPPORTED_IMAGE_FORMATS{ 
	NOT_SUPPORTED,
	SUP_BMP,
	SUP_JPG 
};

/*
 Проверяет, поддерживается ли формат изображения textureName 
 приложением. Критерием служит наличие в имени 
 файла точки, после которой идёт один из форматов,
 опреденных в перечислении SUPPORTED_IMAGE_FORMATS(например,
 filename.jpg).
 Если формат поддерживается, то возвращается соответствующая
 константа из SUPPORTED_IMAGE_FORMATS,а если нет то - NOT_SUPPORTED(0).
*/

int isSupTexture(const char* textureName);

#endif // HELPERFUNS_H
