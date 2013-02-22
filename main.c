#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "HelperFuns.h"
#include "Math.h"
#include "DirectoryWalk.h"
#include "StrList.h"

GLuint winWidth  = 800;
GLuint winHeight = 600;
GLfloat imageScale = 1.0f; // Коэффициент масштабирования( [0.1, 1.0] ).

Mat4x4 worldMat, viewMat, orthoMat, PVW;
Vector3f target = {0,0,0}, eye = {1,0,0}, up = {0,1,0};

GLuint verBuffer;
GLuint shader;

// Ссылки на переменные из шейдера.
GLuint winWidthID;
GLuint winHeightID;
GLuint imageWidthID;  
GLuint imageHeihgtID;
GLuint imageSmp;
GLuint PVWID;

// Отображаемая текстура.
TextureInfo gTexInfo = {0,0,0};

void Render();

extern Display* display;
Window win;

static void MouseWheelHandler(int pos);
static void MousePosHandler(int x, int y);

// Функция для сбрасывания параметров камеры(положение и т.д).
static void ResetCamera();

GLboolean isLMBPressed = GL_FALSE;

int main(int argc, char *argv[]){
	win = createWindow(winWidth, winHeight, "Picture");
	if( win == 0 ){
		fprintf(stderr, "Failed to create window.\n");
		return 0;
	}

	GLXContext glctx_33 = createOpenGLContext(3,3);
	if( glctx_33 == 0 ){
		fprintf(stderr, "Failed to create OpenGL context.\n");
		return 0;
	}

	printf("Making context current.\n");
	glXMakeCurrent( display, win, glctx_33 );

	/*openWalkDir("/home/yunus/Pictures/");*/
	openWalkDir("./images");
	/*openWalkDir("/home/yunus/Desktop/100CANON");*/
	/*openWalkDir("/media/Disc_D/Copy_E/Photo/National Geographic/National Geographic 2011");*/

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

	glGenBuffers(1, &verBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_coords), vertices_coords, GL_STATIC_DRAW);

	// Создаём шейдер.
	shader = CreateShader("vertex_shader.vs", "fragment_shader.fs");
	if( shader == 0 ){
		fprintf(stderr, "%s\n", "Shader isn't created.");
		return -1;
	}

	winWidthID    = glGetUniformLocation(shader, "winWidth");
	winHeightID   = glGetUniformLocation(shader, "winHeight");
	imageWidthID  = glGetUniformLocation(shader, "imageWidth");
	imageHeihgtID = glGetUniformLocation(shader, "imageHeight");

	imageSmp = glGetUniformLocation(shader, "Image");

	// Матрицы.
	Mat4x4Identity(&worldMat);
	Mat4x4View(&viewMat, &eye, &target, &up);
	Mat4x4Ortho(&orthoMat, winWidth/(float)winHeight, 1, 0.1f, 10.0f);
	Mat4x4Mult(&PVW, &viewMat, &worldMat);
	Mat4x4Mult(&PVW, &orthoMat, &PVW);

	PVWID = glGetUniformLocation(shader, "PVW");

	glViewport(0,0, winWidth, winHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	XEvent xev;
	XWindowAttributes wa;
	// Основной цикл приложения.
	while(1){
		XNextEvent(display, &xev);
		if( xev.type == Expose ){
			XGetWindowAttributes(display, win, &wa);
			// При изменении размеров окна необходимо
			// изменить координаты отображаемой области.
		 	float new_r = wa.width/(float)wa.height;
		 	for(int i = 2; i <= 17; i += 3)
				vertices_coords[i] *= new_r/r;
			r = new_r;

			glDeleteBuffers(1, &verBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, verBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_coords), vertices_coords, GL_STATIC_DRAW);

			glViewport(0,0, wa.width, wa.height);
			winWidth = wa.width;
			winHeight = wa.height;
			Mat4x4Ortho(&orthoMat, winWidth/(float)winHeight*imageScale, 1*imageScale, 0.1f, 10.0f);
			Render();
		}

		if( xev.type == ButtonPress ){
			// Button1 - LMB
			// Button3 - RMB
			// Button4 - Wheel forward
			// Button5 - Wheel backward
			if( xev.xbutton.button == Button1 )
				isLMBPressed = GL_TRUE;

			if( xev.xbutton.button == Button3 ){
				ResetCamera();
				Render();
			}

			static int pos = 0;
			if( xev.xbutton.button == Button4 ){
				pos++;
				MouseWheelHandler( pos );
				Render();
			}

			if( xev.xbutton.button == Button5 ){
				pos--;
				MouseWheelHandler( pos );
				Render();
			}
		}

		if( xev.type == ButtonRelease ){
			if( xev.xbutton.button == Button1 )
				isLMBPressed = GL_FALSE;
		}

		static int skipOne = 0;
		if( xev.type == MotionNotify ){
			if( skipOne == 0 ){
				MousePosHandler( xev.xmotion.x, xev.xmotion.y );
				if( isLMBPressed )
					Render();
			}
			skipOne++;
			if( skipOne > 1 )
				skipOne = 0;
		}

		struct timeval start, end;
		if( xev.type == KeyPress ){
			if( XLookupKeysym(&xev.xkey, 0) == XK_Escape )
				break;

			if( XLookupKeysym(&xev.xkey, 0) == XK_Right ){
				gettimeofday(&start, NULL);

				ResetCamera();
				glDeleteTextures(1, &gTexInfo.textureID);
				gTexInfo = getNextImage();

				gettimeofday(&end, NULL);
				double diff = (end.tv_sec + end.tv_usec/1000000.0) - (start.tv_sec + start.tv_usec/1000000.0);
				printf("%.2f s.\n", diff);

				Render();
			}

			if( XLookupKeysym(&xev.xkey, 0) == XK_Left ){
				ResetCamera();
				glDeleteTextures(1, &gTexInfo.textureID);
				gTexInfo = getPrevImage();
				Render();
			}

		}
	}

	// Очистка.
  	glXMakeCurrent( display, 0, 0 );
  	glXDestroyContext( display, glctx_33 );
  	XDestroyWindow( display, win );
  	XCloseDisplay( display );

	glDeleteBuffers(1, &verBuffer);
	glDeleteProgram(shader);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteTextures(1, &gTexInfo.textureID);
	closeWalkDir();

	return 0;
}

static void MouseWheelHandler(int pos){
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

static void MousePosHandler(int x, int y){
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

void Render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4x4View(&viewMat, &eye, &target, &up);
	Mat4x4Mult(&PVW, &viewMat, &worldMat);
	Mat4x4Mult(&PVW, &orthoMat, &PVW);
	// Используем созданный нами шейдер для рендеринга.
	glUseProgram(shader);
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

	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(3*6*sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glXSwapBuffers( display, win );
}
