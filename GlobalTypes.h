#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

// Поддерживаемые форматы изображений.
enum SUPPORTED_IMAGE_FORMATS{ 
	NOT_SUPPORTED,
	SUP_BMP,
	SUP_JPG 
};

// Структура, которая хранит окно и некоторые его параметры.
typedef struct{
	Display* 	 display;
	GLXContext	 GLContext;
	Window 		 win;
	GLuint 		 width;
	GLuint 		 height;
	GLfloat 	 ratio; 	  // width/height.
	GLboolean 	 isRunning;
	GLboolean 	 isLMBPressed;
} WindowInfo;

// Структура, описывающая текстуру.
typedef struct{
	GLuint textureID;
	GLuint width;
	GLuint height;
	char*  name;
} TextureInfo;

#endif // GLOBAL_TYPES_H
