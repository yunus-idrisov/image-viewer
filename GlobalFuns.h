#ifndef GLOBAL_FUNS_H
#define GLOBAL_FUNS_H
#include <FreeImage.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include "DirectoryWalk.h"
#include "GlobalTypes.h"
#include "Math.h"

// Создаёт окно. При ошибке возвращается 0.
Window CreateWindow(GLuint width, GLuint height, const char* title);

// Создаёт контекст OpenGL. При ошибке возвращается 0.
GLXContext CreateOpenGLContext(int ver_major, int ver_minor);

// Функция CreateShader создаёт шейдерную программу из 
// файлов вершинного и фрагментного шейдеров.
// Если произошла ошибка возвращается 0.
GLuint CreateShader(const char *vertex_shader_path, const char *fragment_shader_path);

// Функция CreateShaderStr создаёт шейдерную программу из 
// строк,содержащих вершинный и фрагментный шейдеры.
// Если произошла ошибка возвращается 0.
GLuint CreateShaderStr(const char *vertex_shader, const char *fragment_shader);

/*
 Создаёт текстуру OpenGL и возвращает её идентификатор,
 ширину и высоту.
 Имя текстуры объязательно должно содержать расширение.
 Если произошла ошибка, то в поле textureID 
 структуры TextureInfo записывается 0.
*/
TextureInfo LoadTexture(const char *texturePath);

/*
 * Чтобы распараллелить загрузку текстуры делим функцию
 * LoadTexture на две части: функция GetFIBITMAP(...) 
 * возвращает указатель на структуру FIBITMAP, переданного
 * в аргументе изображения. Далее из этой структуры
 * функция CreateGLTexture(...) создаёт и возвращает 
 * структуру TextureInfo, которая и содержит созданную
 * текстуру.
 * Функция GetFIBITMAP(...) возвращает 0, если по каким-
 * либо причинам не удалось создать FIBITMAP.
 * CreateGLTexture(...) при возникновении ошибки устанавливает
 * в поле textureID структуры TextureInfo значение 0.
 *
 * Необходимость деления функции LoadTexture(...) связана
 * с тем, что контекст OpenGL в определенный момент времени
 * связан только с одним потоком. Следовательно в другом потоке
 * невозможно вызвать функции OpenGL. Функция CreateGLTexture(...)
 * вызывается в основном потоке, а GetFIBITMAP(...) в побочном.
 */
FIBITMAP*	  GetFIBITMAP(const char *texturePath);
TextureInfo	  CreateGLTexture(FIBITMAP* bitmap);

/*
 Проверяет, поддерживается ли формат изображения textureName 
 приложением. Критерием служит наличие в имени 
 файла точки, после которой идёт один из форматов,
 опреденных в перечислении SUPPORTED_IMAGE_FORMATS(например,
 filename.jpg).
 Если формат поддерживается, то возвращается соответствующая
 константа из SUPPORTED_IMAGE_FORMATS,а если нет то - NOT_SUPPORTED(0).
*/
int IsSupTexture(const char* textureName);

// Оптимизация ??????????????
// Время, отсчитваемое от Epoch.
GLdouble GetTime();

// Инициализация.
int  InitAppliction(const char* winName,
					GLuint width,
					GLuint height,
					GLuint glversion_major,
					GLuint glversion_minor);
// Рендер.
void Render();
// Обработчик событий.
void EventHandler(XEvent xev);
// Очистка.
void CleanUp();

void MouseWheelHandler(int wheelPos);
void MousePosHandler(int x, int y);

// Функция, которая осуществляет плавное 
// перелиствание. Если входной параметр GL_TRUE, то
// загружается следующее изображение.
void AnimateImageShow(GLboolean next);

// Функция для сбрасывания параметров камеры(положение и т.д).
void ResetCamera();

// Создаёт vertex array object и вершинный буфер.
void CreateVertexArray();

#endif // GLOBAL_FUNS_H
