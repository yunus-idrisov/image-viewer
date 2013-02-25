#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "HelperFuns.h"
#include "Math.h"
#include "DirectoryWalk.h"

WindowInfo winInfo = { 0, 0, 0, 800, 600, 800.0f/600.0f, GL_TRUE, GL_FALSE };

Mat4x4 worldMat, viewMat, orthoMat, PVW;
Vector3f target = {0,0,0}, eye = {1,0,0}, up = {0,1,0};
void ResetCamera();		// Функция для сбрасывания параметров камеры(положение и т.д).

GLuint vao;
GLuint verBuffer;
void crtVertexArray();	// Создаёт vertex array object и вершинный буфер.

GLuint shader;
// Ссылки на переменные из шейдера.
GLuint winWidthID;
GLuint winHeightID;
GLuint imageWidthID;  
GLuint imageHeihgtID;
GLuint imageSmp;
GLuint PVWID;


TextureInfo gTexInfo = {0,0,0}; 				// Отображаемая текстура.
GLfloat imageScale = 1.0f;						// Коэффициент масштабирования( [0.1, 1.0] ).

int  InitAppliction(const char* winName,		// Инициализация.
					GLuint width,
					GLuint height,
					GLuint glversion_major,
					GLuint glversion_minor);

void Render();		   							// Рендер.
void EventHandler(XEvent xev);  				// Обработчик событий.
void CleanUp(); 								// Очистка.

void MouseWheelHandler(int wheelPos);
void MousePosHandler(int x, int y);

int main(int argc, char *argv[]){
	if( InitAppliction("Image", 800, 600, 3, 3) == 0 ){
		fprintf(stderr, "Initialization failed.\n");
		return 0;
	}

	/*openWalkDir("/home/yunus/Pictures/");*/
	openWalkDir("./images");
	/*openWalkDir("/home/yunus/Desktop/100CANON");*/
	/*openWalkDir("/media/Disc_D/Copy_E/Photo/National Geographic/National Geographic 2011");*/

	XEvent xev;
	// Основной цикл приложения.
	while( winInfo.isRunning ){
		XNextEvent(winInfo.display, &xev);
		EventHandler( xev );
	}

	closeWalkDir();
	CleanUp();
	return 0;
}

int  InitAppliction(const char* winName,
					GLuint width,
					GLuint height,
					GLuint glversion_major,
					GLuint glversion_minor)
{
	// Создаём окно.
	winInfo.width = width;
	winInfo.height = height;
	winInfo.ratio = width/(float)height;
	winInfo.win = createWindow(winInfo.width, winInfo.height, winName);
	if( winInfo.win == 0 ){
		fprintf(stderr, "Failed to create window.\n");
		return 0;
	}

	// Создаём контекст OpenGL.
	winInfo.GLContext = createOpenGLContext(glversion_major, glversion_minor);
	if( winInfo.GLContext == 0 ){
		fprintf(stderr, "Failed to create OpenGL context.\n");
		return 0;
	}

	// Делаем контекст текущим.
	printf("Making context current.\n");
	glXMakeCurrent( winInfo.display, winInfo.win, winInfo.GLContext );

	// Создаём vertex array object и буфер для вершин.
	crtVertexArray();

	// Создаём шейдер.
	shader = CreateShader("vertex_shader.vs", "fragment_shader.fs");
	if( shader == 0 ){
		fprintf(stderr, "%s\n", "Shader isn't created.");
		return 0;
	}

	// Извлечение переменных из шейдера.
	winWidthID    = glGetUniformLocation(shader, "winWidth");
	winHeightID   = glGetUniformLocation(shader, "winHeight");
	imageWidthID  = glGetUniformLocation(shader, "imageWidth");
	imageHeihgtID = glGetUniformLocation(shader, "imageHeight");
	imageSmp 	  = glGetUniformLocation(shader, "Image");
	PVWID 		  = glGetUniformLocation(shader, "PVW");

	// Инициализируем матрицы.
	Mat4x4Identity(&worldMat);
	Mat4x4View(&viewMat, &eye, &target, &up);
	Mat4x4Ortho(&orthoMat, winInfo.ratio, 1, 0.1f, 10.0f);
	Mat4x4Mult(&PVW, &viewMat, &worldMat);
	Mat4x4Mult(&PVW, &orthoMat, &PVW);

	glViewport(0,0, winInfo.width, winInfo.height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	return 1;
}

void Render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4x4View(&viewMat, &eye, &target, &up);
	Mat4x4Mult(&PVW, &viewMat, &worldMat);
	Mat4x4Mult(&PVW, &orthoMat, &PVW);
	// Используем созданный нами шейдер для рендеринга.
	glUseProgram(shader);
	glUniformMatrix4fv(PVWID, 1, GL_TRUE, PVW.m);
	glUniform1i(winWidthID, winInfo.width);
	glUniform1i(winHeightID, winInfo.height);
	glUniform1i(imageWidthID, gTexInfo.width);
	glUniform1i(imageHeihgtID, gTexInfo.height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexInfo.textureID);
	glUniform1i(imageSmp, 0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glXSwapBuffers( winInfo.display, winInfo.win );
}

void EventHandler(XEvent xev){
	XWindowAttributes wa;
	static int wheelPos = 0;
	static int skipOne = 0;
	struct timeval start, end;

	switch( xev.type ){
		case Expose :
			XGetWindowAttributes(winInfo.display, winInfo.win, &wa);
			glViewport(0,0, wa.width, wa.height);
			winInfo.width = wa.width;
			winInfo.height = wa.height;
			winInfo.ratio = wa.width/(float)wa.height;
			Mat4x4Ortho(&orthoMat, winInfo.ratio*imageScale, 1*imageScale, 0.1f, 10.0f);
			Render();
			break;

		case ButtonPress : 
			switch( xev.xbutton.button ){
				case Button1 : // LMB
					winInfo.isLMBPressed = GL_TRUE;
					break;

				case Button3 : // RMB
					ResetCamera();
					Render();
					break;

				case Button4 : // Wheel forward
					wheelPos++;
					MouseWheelHandler( wheelPos );
					Render();
					break;

				case Button5 : // Wheel backward
					wheelPos--;
					MouseWheelHandler( wheelPos );
					Render();
					break;
			};
			break;

		case ButtonRelease :
			if( xev.xbutton.button == Button1 )
				winInfo.isLMBPressed = GL_FALSE;
			break;

		// Mouse motion.
		case MotionNotify :
			if( skipOne == 0 ){
				MousePosHandler( xev.xmotion.x, xev.xmotion.y );
				if( winInfo.isLMBPressed )
					Render();
			}
			skipOne++;
			if( skipOne > 1 )
				skipOne = 0;
			break;
			
		case KeyPress :
			switch( XLookupKeysym(&xev.xkey, 0) ){
				case XK_Escape :
					winInfo.isRunning = GL_FALSE;
					break;

				case XK_Right :
					gettimeofday(&start, NULL);

					ResetCamera();
					glDeleteTextures(1, &gTexInfo.textureID);
					gTexInfo = getNextImage();

					gettimeofday(&end, NULL);
					double diff = (end.tv_sec + end.tv_usec/1000000.0) - (start.tv_sec + start.tv_usec/1000000.0);
					printf("%.2f s.\n", diff);

					Render();
					break;

				case XK_Left :
					ResetCamera();
					glDeleteTextures(1, &gTexInfo.textureID);
					gTexInfo = getPrevImage();
					Render();
					break;
			}
			break;
	};
}

void crtVertexArray(){
	glGenVertexArrays(1, &vao);
	glBindVertexArray( vao );

	float vertices_coords[] = {
		 0.0f,  1.0f/2, -1.0f/2,
		 0.0f,  1.0f/2,  1.0f/2,
		 0.0f, -1.0f/2,  1.0f/2,
		 0.0f,  1.0f/2, -1.0f/2,
		 0.0f, -1.0f/2,  1.0f/2,
		 0.0f, -1.0f/2, -1.0f/2,

		 1.0f, 1.0f,
		 0.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 0.0f
	};

	glGenBuffers(1, &verBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_coords), vertices_coords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(3*6*sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void MouseWheelHandler(int wheelPos){
	static int prevPos = 1;
	if( wheelPos > prevPos )
		imageScale -= 0.1f;
	else
		imageScale += 0.1f;
	if( imageScale < 0.1f )
		imageScale = 0.1f;
	if( imageScale > 1.0f )
		imageScale = 1.0f;
	Mat4x4Ortho(&orthoMat, winInfo.ratio*imageScale, 1*imageScale, 0.1f, 10.0f);
	prevPos = wheelPos;
}

void MousePosHandler(int x, int y){
	// Перемещение изображения.
	static int prev_x = 0, prev_y = 0;

	float r = winInfo.ratio;
	float horOffset = (x - prev_x)/(float)winInfo.width*r*imageScale;
	float verOffset = (y - prev_y)/(float)winInfo.height*1.0f*imageScale;
	if( winInfo.isLMBPressed ){
		target.z += horOffset;
		eye.z += horOffset;
		target.y += verOffset;
		eye.y += verOffset;
	}
	prev_x = x;
	prev_y = y;
}

void ResetCamera(){
	target.x = target.y = target.z = 0.0f;
	eye.x = 1.0f;
	eye.y = eye.z = 0.0f;
	imageScale = 1.0f;
	Mat4x4Ortho(&orthoMat, winInfo.ratio*imageScale, 1*imageScale, 0.1f, 10.0f);
}

void CleanUp(){
  	glXMakeCurrent( winInfo.display, 0, 0 );
  	glXDestroyContext( winInfo.display, winInfo.GLContext );
  	XDestroyWindow( winInfo.display, winInfo.win );
  	XCloseDisplay( winInfo.display );

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &verBuffer);
	glDeleteProgram(shader);
	glDeleteTextures(1, &gTexInfo.textureID);
}
