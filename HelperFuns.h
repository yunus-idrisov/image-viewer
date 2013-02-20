#ifndef HELPERFUNS_H
#define HELPERFUNS_H
#include <GL/gl.h>
#include <FreeImage.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

Window createWindow(GLuint width, GLuint height, const char* title);
GLXContext createOpenGLContext(int ver_major, int ver_minor);

// Функция CreateShader создаёт шейдерную программу из 
// вершинного и фрагментного шейдеров.
// Если произошла ошибка возвращается 0.
GLuint CreateShader(const char *vertex_shader_path, const char *fragment_shader_path);

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

/*
 * Чтобы распараллелить загрузку текстуры делим функцию
 * loadTexture на две части: функция getFIBITMAP(...) 
 * возвращает указатель на структуру FIBITMAP, переданного
 * в аргументе изображения. Далее из этой структуры
 * функция createGLTexture(...) создаёт и возвращает 
 * структуру TextureInfo, которая и содержит созданную
 * текстуру.
 * Функция getFIBITMAP(...) возвращает 0, если по каким-
 * либо причинам не удалось создать FIBITMAP.
 * createGLTexture(...) при возникновении ошибки устанавливает
 * в поле textureID структуры TextureInfo значение 0.
 *
 * Необходимость деления функции loadTexture(...) связана
 * с тем, что контекст OpenGL в определенный момент времени
 * связан только с одним потоком. Следовательно в другом потоке
 * невозможно вызвать функции OpenGL. Функция createGLTexture(...)
 * вызывается в основном потоке, а getFIBITMAP(...) в побочном.
 */
FIBITMAP*	  getFIBITMAP(const char *texturePath);
TextureInfo	  createGLTexture(FIBITMAP* bitmap);

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
