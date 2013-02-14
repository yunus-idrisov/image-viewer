#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include </usr/local/include/GL/glfw.h>
#include "HelperFuns.h"
#include "Math.h"
#include "DirectoryWalk.h"
#include "StrList.h"

GLuint winWidth  = 800;
GLuint winHeight = 600;
GLfloat imageScale = 1.0f; // Коэффициент масштабирования( [0.1, 1.0] ).

Mat4x4 worldMat, viewMat, orthoMat;
Vector3f target = {0,0,0}, eye = {1,0,0}, up = {0,1,0};

TextureInfo gTexInfo = {0,0,0};

static void GLFWCALL KeyboardInputHandler(int key, int state);
static void GLFWCALL MouseWheelHandler(int pos);
static void GLFWCALL MouseButtonsHandler(int button, int action);
static void GLFWCALL MousePosHandler(int x, int y);

// Функция для сбрасывания параметров камеры(положение и т.д).
static void ResetCamera();

GLboolean isRunning = GL_TRUE;
GLboolean isLMBPressed = GL_FALSE;

int main(int argc, char *argv[]){
	/*openWalkDir("/home/yunus/Pictures/");*/
	if( !glfwInit() ){
		fprintf(stderr, "%s\n", "Failed to initialize GLFW.");
		return -1;
	}
	// Параметры OpenGL.
	// Antialiasing.
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	// OpenGL контекст версии 3.3
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	// Используем только новые возможности OpenGL.
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// w, h, rbits, gbits, bbits, abits, depth bits, stencil bits, fullscreen/windowed.
	// Если 0, то GLFW выберет значение по умолчанию или запретит.
	if( !glfwOpenWindow(winWidth, winHeight, 0,0,0,0, 32, 0, GLFW_WINDOW) ){
		fprintf(stderr, "%s\n", "Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	/*openWalkDir("./images");*/
	/*openWalkDir("/home/yunus/Desktop/100CANON");*/
	openWalkDir("/media/Disc_D/Copy_E/Photo/National Geographic/National Geographic 2011");

	glfwSetWindowTitle("Framebuffer.");
	glfwSetKeyCallback( KeyboardInputHandler );
	glfwSetMouseButtonCallback( MouseButtonsHandler );
	glfwSetMousePosCallback( MousePosHandler );
	glfwSetMouseWheelCallback( MouseWheelHandler );
	glfwEnable(GLFW_KEY_REPEAT);

	// Context created. Going further.

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLfloat r = winWidth/(float)winHeight;
	GLfloat h = 1.0f;
	// Создаём буфер для вершин.
	float vertices_coords[] = {
		 0.0f, h/2, -r/2,
		 0.0f, h/2,  r/2,
		 0.0f,-h/2,  r/2,
		 0.0f, h/2, -r/2,
		 0.0f,-h/2,  r/2,
		 0.0f,-h/2, -r/2,

		 1.0f, 1.0f,
		 0.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 0.0f
	};

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_coords), vertices_coords, GL_STATIC_DRAW);

	// Создаём шейдер.
	GLuint shProgram = CreateProgram("vertex_shader.vs", "fragment_shader.fs");
	if( shProgram == 0 ){
		fprintf(stderr, "%s\n", "Shader isn't created.");
		glfwTerminate();
		return -1;
	}

	GLuint winWidthID    = glGetUniformLocation(shProgram, "winWidth");
	GLuint winHeightID   = glGetUniformLocation(shProgram, "winHeight");
	GLuint imageWidthID  = glGetUniformLocation(shProgram, "imageWidth");
	GLuint imageHeihgtID = glGetUniformLocation(shProgram, "imageHeight");

	GLuint imageSmp = glGetUniformLocation(shProgram, "Image");

	// Матрицы.
	Mat4x4Identity(&worldMat);
	Mat4x4View(&viewMat, &eye, &target, &up);
	Mat4x4Ortho(&orthoMat, winWidth/(float)winHeight, 1, 0.1f, 10.0f);
	Mat4x4 PVW;
	Mat4x4Mult(&PVW, &viewMat, &worldMat);
	Mat4x4Mult(&PVW, &orthoMat, &PVW);

	GLuint PVWID = glGetUniformLocation(shProgram, "PVW");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Основной цикл продолжается пока не нажата 
	// клавиша Esc или окно не закрыто.
	while( isRunning ){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Mat4x4View(&viewMat, &eye, &target, &up);
		Mat4x4Mult(&PVW, &viewMat, &worldMat);
		Mat4x4Mult(&PVW, &orthoMat, &PVW);
		// Используем созданный нами шейдер для рендеринга.
		glUseProgram(shProgram);
		glUniformMatrix4fv(PVWID, 1, GL_TRUE, PVW.m);
		glUniform1i(winWidthID, winWidth);
		glUniform1i(winHeightID, winHeight);
		glUniform1i(imageWidthID, gTexInfo.width);
		glUniform1i(imageHeihgtID, gTexInfo.height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTexInfo.textureID);
		glUniform1i(imageSmp, 0);

		// Первый параметр передаваемый в шейдер.
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(3*6*sizeof(GLfloat)));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		glfwSwapBuffers();
		if( glfwGetWindowParam(GLFW_OPENED) != GL_TRUE )
			isRunning = GL_FALSE;
	}

	// Очистка.
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteProgram(shProgram);
	glDeleteVertexArrays(1, &VertexArrayID);
	glfwTerminate();
	closeWalkDir();

	return 0;
}

static void GLFWCALL KeyboardInputHandler(int key, int state){
	struct timeval start, end;

	switch(key){
		case GLFW_KEY_ESC : 
			if( state == GLFW_RELEASE )
				isRunning = GL_FALSE;
			break;

		case GLFW_KEY_RIGHT :
			if( state == GLFW_RELEASE ){
				gettimeofday(&start, NULL);

				ResetCamera();
				// Удаляем предыдущую текстуру.
				glDeleteTextures(1, &gTexInfo.textureID);
				// Загружаем следующую.
				gTexInfo = getNextImage();

				// Time to load next texture.
				gettimeofday(&end, NULL);
				double diff = (end.tv_sec + end.tv_usec/1000000.0) - (start.tv_sec + start.tv_usec/1000000.0);
				printf("%.2f s.\n", diff);
			}
			break;

		case GLFW_KEY_LEFT :
			if( state == GLFW_RELEASE ){
				ResetCamera();
				// Удаляем предыдущую текстуру.
				glDeleteTextures(1, &gTexInfo.textureID);
				// Загружаем предыдущую.
				gTexInfo = getPrevImage();
			}
			break;
	}
}

static void GLFWCALL MouseWheelHandler(int pos){
	static int prevPos = 1;
	if( pos > prevPos )
		imageScale -= 0.1f;
	else
		imageScale += 0.1f;
	if( imageScale < 0.1f )
		imageScale = 0.1f;
	if( imageScale > 1.0f )
		imageScale = 1.0f;
	Mat4x4Ortho(&orthoMat, winWidth/(float)winHeight*imageScale, 1*imageScale, 0.1f, 10.0f);
	prevPos = pos;
}

static void GLFWCALL MouseButtonsHandler(int button, int action){
	if( button == GLFW_MOUSE_BUTTON_LEFT )
		if( action == GLFW_PRESS )
			isLMBPressed = GL_TRUE;
		else
			isLMBPressed = GL_FALSE;

	if( button == GLFW_MOUSE_BUTTON_RIGHT )
		ResetCamera();
}

static void GLFWCALL MousePosHandler(int x, int y){
	// Перемещение изображения.
	static int prev_x = 0, prev_y = 0;

	float r = winWidth/(float)winHeight;
	float horOffset = (x - prev_x)/(float)winWidth*r*imageScale;
	float verOffset = (y - prev_y)/(float)winHeight*1.0f*imageScale;
	if( isLMBPressed ){
		target.z += horOffset;
		eye.z += horOffset;
		target.y += verOffset;
		eye.y += verOffset;
	}
	prev_x = x;
	prev_y = y;
}

static void ResetCamera(){
	target.x = target.y = target.z = 0.0f;
	eye.x = 1.0f;
	eye.y = eye.z = 0.0f;
	imageScale = 1.0f;
	Mat4x4Ortho(&orthoMat, winWidth/(float)winHeight*imageScale, 1*imageScale, 0.1f, 10.0f);
}
