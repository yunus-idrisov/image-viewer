#include "GlobalFuns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "Shaders.h"
#include "icon_bitmap"

extern WindowInfo winInfo;

extern Mat4x4 worldMat, viewMat, orthoMat, PVW;
extern Vector3f target, eye, up;

extern GLfloat   zOffset, yOffset;
extern GLboolean isAnimationEnable;

extern GLuint vao;
extern GLuint verBuffer;

extern GLuint shader;
extern GLuint winWidthID;
extern GLuint winHeightID;
extern GLuint imageWidthID;  
extern GLuint imageHeihgtID;
extern GLuint imageSmp;
extern GLuint zOffsetID;
extern GLuint yOffsetID;
extern GLuint PVWID;

extern TextureInfo gTexInfo;
extern GLfloat imageScale;

#define MAX_SHADER_SIZE		8192
#define MAX_LINE_SIZE		128

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

static GLXFBConfig bestFbc;

Window CreateWindow(GLuint width, GLuint height, const char* title){
	Window win;

	if( (winInfo.display = XOpenDisplay(NULL)) == NULL ){
		fprintf(stderr, "Cannot connect to X server %s\n", XDisplayName(NULL) );
		return 0;
	}

	int defScreen = XDefaultScreen( winInfo.display );

	// Извлечение версии GLX.
	int glx_major, glx_minor;
	if( !glXQueryVersion(winInfo.display, &glx_major, &glx_minor) ){
		fprintf(stderr, "Query GLX version error.\n");
		return 0;
	}

	// Версии GLX ниже 1.3 не проходят.
	if( glx_minor < 3 ){
		if( glx_major <= 1 ){
			fprintf(stderr, "Invalid GLX version(< 1.3).\n");
			return 0;
		}
	}else
		if( glx_major < 1 ){
			fprintf(stderr, "Invalid GLX version(< 1.3).\n");
			return 0;
		}

	int visual_attributes[] = {
		GLX_X_RENDERABLE	, True,
		GLX_DRAWABLE_TYPE	, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE		, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE	, GLX_TRUE_COLOR,
		GLX_RED_SIZE		, 8,
		GLX_GREEN_SIZE		, 8,
		GLX_BLUE_SIZE		, 8,
		GLX_ALPHA_SIZE		, 8,
		GLX_DEPTH_SIZE		, 24,
		GLX_STENCIL_SIZE	, 8,
		GLX_DOUBLEBUFFER	, True,
		GLX_SAMPLE_BUFFERS	, 1,
		GLX_SAMPLES			, 4,
		None
	};

	printf("Getting matching framebuffer(FB) configs.\n");
	int fbcount = 0;
	GLXFBConfig* fbc = glXChooseFBConfig(winInfo.display, defScreen, visual_attributes, &fbcount);
	if( fbc == NULL ){
		printf("Failed to retrieve a FB configs.\n");
		return 0;
	}
	printf("Found %d matching FB configs.\n", fbcount);

	// Выбираем  FB с наибольшим количеством сэмплов.
	printf("Getting XVisualInfos:\n");
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for(int i = 0; i < fbcount; i++){
		XVisualInfo* vi = glXGetVisualFromFBConfig(winInfo.display, fbc[i]);
		if( vi ){
			int samp_buf, samples;
			glXGetFBConfigAttrib( winInfo.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib( winInfo.display, fbc[i], GLX_SAMPLES, &samples);
			printf("Matching fbconfig %d, visual ID 0x%x: SAMPLE_BUFFERS = %d,"
				   " SAMPLES = %d\n", i, vi->visualid, samp_buf, samples);

			// Лучший fbconfig.
			if( best_fbc < 0 || samp_buf && samples > best_num_samp ){
				best_fbc = i;
				best_num_samp = samples;
			}
			// Хучший fbconfig.
			if( worst_fbc < 0 || !samp_buf || samples < worst_num_samp ){
				worst_fbc = i;
				worst_num_samp = samples;
			}
		}
		XFree(vi);
	}

	// Копируем лучший fbconfig.
	bestFbc = fbc[ best_fbc ];

	// Освобождаем память, выделенную glXChooseFBConfig(...).
	XFree(fbc);

	// Выбираем лучший visual.
	XVisualInfo *vi = glXGetVisualFromFBConfig(winInfo.display, bestFbc);
	printf("Chosen visual ID = 0x%x\n", vi->visualid);

	// Создаём colormap.
	printf("Creating colormap.\n");
	Colormap cmap;
	cmap = XCreateColormap( winInfo.display, 
							RootWindow(winInfo.display, vi->screen),
							vi->visual,
							AllocNone );

	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask | ExposureMask | KeyReleaseMask | 
					 PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

	printf("Creating window.\n");
	win = XCreateWindow(winInfo.display, RootWindow(winInfo.display, vi->screen),
							   0, 0, width, height, 0, vi->depth, InputOutput,
							   vi->visual,
							   CWBorderPixel | CWColormap | CWEventMask, &swa);

	XFree(vi);
	XFreeColormap( winInfo.display, cmap );
	if( win == NULL ){
		fprintf(stderr, "Failed to create window.\n");
		return 0;
	}

	XStoreName(winInfo.display, win, title);

	// Window hints.
	XSizeHints* size_hints;
	XWMHints*	wm_hints;
	XClassHint*	class_hints;

	if( !(size_hints = XAllocSizeHints()) ){
		fprintf(stderr, "XAllocSizeHints(): failed to allocate memory.");
		return 0;
	}

	if( !(wm_hints = XAllocWMHints()) ){
		fprintf(stderr, "XAllocWMHints(): failed to allocate memory.");
		return 0;
	}

	if( !(class_hints = XAllocClassHint()) ){
		fprintf(stderr, "XAllocClassHint(): failed to allocate memory");
		return 0;
	}

	Pixmap icon_pixmap;

	icon_pixmap = XCreateBitmapFromData(winInfo.display, win,
			icon_bitmap_bits,
			icon_bitmap_width,
			icon_bitmap_height);

	size_hints->flags = PPosition | PSize | PMinSize;
	size_hints->min_width = 300;
	size_hints->min_height = 200;

	char* window_name = title;
	char* icon_name = "basicwin";
	XTextProperty windowName;
	XTextProperty iconName;

	if( XStringListToTextProperty(&window_name, 1, &windowName) == 0 ){
		fprintf(stderr, "Structure allocation for windowName failed.\n");
		return 0;
	}

	if( XStringListToTextProperty(&icon_name, 1, &iconName) == 0 ){
		fprintf(stderr, "Structure allocation for iconName failed.\n");
		return 0;
	}

	wm_hints->initial_state = NormalState;
	wm_hints->input = True;
	wm_hints->icon_pixmap = icon_pixmap;
	wm_hints->flags = StateHint | IconPixmapHint | InputHint;

	class_hints->res_name = "Picture";
	class_hints->res_class = "Basicwin";
	XSetWMProperties( winInfo.display, win, &windowName, &iconName, NULL, 0, size_hints, wm_hints, class_hints);

	printf("Mapping window.\n");
	XMapWindow( winInfo.display, win );

	return win;
}

static int ctxErrorOccured = 0;
int ctxErrorHandler(Display* dpy, XErrorEvent* ev){
	ctxErrorOccured = 1;
	return 0;
}

GLXContext CreateOpenGLContext(int ver_major, int ver_minor){
	GLXContext glctx = 0;

	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, 
														 GLXFBConfig, 
														 GLXContext, 
														 Bool, 
														 const int*);

	// Извлекаем расширение glXCreateContextAttribsARB.
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
		glXGetProcAddress( (const GLubyte*)"glXCreateContextAttribsARB" );

	// Если расширение не удалось получить, то мы 
	// не можем создать контекст(выше 3.0 ?).
	if( !glXCreateContextAttribsARB ){
		printf("Extension glXCreateContextAttribsARB not found."
			   "Cannot create OpenGL context.\n");
		return glctx;
	}

	// Устанавливаем новый обработчик ошибок Xlib, чтобы программа не 
	// завершилась, если не удастся создать контекст OpenGL.
	ctxErrorOccured = 0;
	int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, ver_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, ver_minor,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	printf("Creating OpenGL context.\n");
	glctx = glXCreateContextAttribsARB(winInfo.display, bestFbc, 0, True, context_attribs);

	XSync( winInfo.display, False );
	if( !ctxErrorOccured && glctx ){
		printf("OpenGL %d.%d context created.\n", ver_major, ver_minor);
	}
	else{
		printf("Cannot create OpenGL context.\n");
		XSetErrorHandler( oldHandler );
		return glctx;
	}

	XSync( winInfo.display, False );

	// Восстанавливаем начальный обработчик ошибок Xlib.
	XSetErrorHandler( oldHandler );
	if( ctxErrorOccured || !glctx ){
		printf("Cannot create OpenGL context.\n");
		return glctx;
	}

	// Проверяем является созданный контекст direct контекстом.
	if( !glXIsDirect( winInfo.display, glctx ) )
		printf("Indirect GLX rendering context obtained\n");
	else
		printf("Direct GLX rendering context obtained.\n");

	return glctx;
}

GLuint CreateShader(const char *vertex_shader_path, const char *fragment_shader_path){
	// Сначала создаём вершинный шейдер.

	// Считываем код вер. шейдера.
	char* ShaderCode = (char*)malloc( sizeof(char) * MAX_SHADER_SIZE );
	FILE *in = fopen(vertex_shader_path, "r");

	if( in == NULL ){
		fprintf(stderr, "Error:cann't open \"%s\" file.\n", vertex_shader_path);
		return 0;
	}
	ShaderCode[0] = '\0';

	char buf[MAX_LINE_SIZE];
	int  lineLength = MAX_LINE_SIZE;
	while( fgets(buf, MAX_LINE_SIZE, in) != NULL ){
		strcat(ShaderCode, buf);
	}
	fclose(in);

	// Компилируем вер. шейдер.
	GLuint verShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(verShader, 1, &ShaderCode, NULL);
	glCompileShader(verShader);

	// Проверём не возникли ли ошибки во время компиляции.
	GLint result;
	int infoLogLength;
	glGetShaderiv(verShader, GL_COMPILE_STATUS, &result);
	if( result == GL_FALSE ){
		glGetShaderiv(verShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(verShader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in \"%s\": %s\n", vertex_shader_path, errMessage);
		free(errMessage);
		free(ShaderCode);
		return 0;
	}

	// Теперь создаём фрагментный шейдер.
	// Считываем код фраг. шейдера.

	ShaderCode[0] = '\0';
	in = fopen(fragment_shader_path, "r");

	if( in == NULL ){
		fprintf(stderr, "Error:cann't open \"%s\" file.\n", fragment_shader_path);
		return 0;
	}

	while( fgets(buf, MAX_LINE_SIZE, in) != NULL ){
		strcat(ShaderCode, buf);
	}
	fclose(in);

	// Компилируем фраг. шейдер.
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &ShaderCode, NULL);
	glCompileShader(fragShader);
	free(ShaderCode);

	// Проверём не возникли ли ошибки во время компиляции.
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	if( result == GL_FALSE ){
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(fragShader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in \"%s\": %s\n", fragment_shader_path, errMessage);
		free(errMessage);
		return 0;
	}

	// Создаём шейдерную программу.
	GLuint shader = glCreateProgram();
	glAttachShader(shader, verShader);
	glAttachShader(shader, fragShader);
	glLinkProgram(shader);

	// Проверём не возникли ли ошибки во время компоновки.
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if( result == GL_FALSE ){
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetProgramInfoLog(shader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in shader: %s\n", errMessage);
		free(errMessage);
		return 0;
	}

	glDeleteShader(verShader);
	glDeleteShader(fragShader);

	return shader;
}

GLuint CreateShaderStr(const char *vertex_shader, const char *fragment_shader){
	// Компилируем вер. шейдер.
	GLuint verShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(verShader, 1, &vertex_shader, NULL);
	glCompileShader(verShader);

	// Проверём не возникли ли ошибки во время компиляции.
	GLint result;
	int infoLogLength;
	glGetShaderiv(verShader, GL_COMPILE_STATUS, &result);
	if( result == GL_FALSE ){
		glGetShaderiv(verShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(verShader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in vertex shader: %s\n", errMessage);
		free(errMessage);
		return 0;
	}

	// Компилируем фраг. шейдер.
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragment_shader, NULL);
	glCompileShader(fragShader);

	// Проверём не возникли ли ошибки во время компиляции.
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	if( result == GL_FALSE ){
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(fragShader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in fragment shader: %s\n", errMessage);
		free(errMessage);
		return 0;
	}

	// Создаём шейдерную программу.
	GLuint shader = glCreateProgram();
	glAttachShader(shader, verShader);
	glAttachShader(shader, fragShader);
	glLinkProgram(shader);

	// Проверём не возникли ли ошибки во время компоновки.
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if( result == GL_FALSE ){
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetProgramInfoLog(shader, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in shader: %s\n", errMessage);
		free(errMessage);
		return 0;
	}

	glDeleteShader(verShader);
	glDeleteShader(fragShader);

	return shader;
}

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char* message);

int IsSupTexture(const char* textureName){
	int sif = NOT_SUPPORTED;

	char *dot_loc = strrchr(textureName, '.');
	// Если файл не содержит точки, то считается, что
	// расширение для него не определено.
	if( dot_loc == NULL ){
		fprintf(stderr, "Error: file \"%s\" doesn't contain format.\n", textureName);
		return sif;
	}// Если точка стоит в конце файла.
	else if( *(dot_loc + 1) == '\0' ){
		fprintf(stderr, "Error: file \"%s\" doesn't contain format.\n", textureName);
		return sif;
	}

	char* Format = malloc(strlen(dot_loc) + 1);
	strcpy(Format, dot_loc + 1);
	// Переводим формат в нижний регистр.
	for(int i = 0; i < strlen(Format); i++)
		if( isupper(Format[i]) )
			Format[i] = tolower(Format[i]);

	if( strcmp(Format, "bmp") == 0 )
		sif = SUP_BMP;
	else if( strcmp(Format, "jpg") == 0 )
		sif = SUP_JPG;

	free(Format);
	return sif;
}

TextureInfo LoadTexture(const char *texturePath){
	// Функция для обработки ошибок.
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	TextureInfo texInfo = {0,0,0};

	FREE_IMAGE_FORMAT fif = 0;

	switch( IsSupTexture(texturePath) ){
		case NOT_SUPPORTED:
			fprintf(stderr, "Error: image format isn't supported yet.\n");
			return texInfo;
			break;
		case SUP_BMP:
			fif = FIF_BMP;
			break;
		case SUP_JPG:
			fif = FIF_JPEG;
			break;
	}

	FIBITMAP* bitmap = FreeImage_Load(fif, texturePath, 0);

	if( bitmap == 0 ){
		fprintf(stderr, "Error: image \"%s\" isn't loaded.\n", texturePath);
		return texInfo;
	}

	GLint width  = FreeImage_GetWidth(bitmap);
	GLint height = FreeImage_GetHeight(bitmap);

	// Pointer to texture colors.
	GLubyte* imgData = (GLubyte*)FreeImage_GetBits(bitmap);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// For bmp, jpg.

	// Compressed image. Less memory but more time to load.
	/*glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imgData);*/

	// Uncompressed image. More memory but less time to load.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imgData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	FreeImage_Unload(bitmap);
	bitmap = 0;
	glBindTexture(GL_TEXTURE_2D, 0);

	texInfo.textureID = texture;
	texInfo.width = width;
	texInfo.height = height;
	return texInfo;
}

FIBITMAP* GetFIBITMAP(const char *texturePath){
	// Функция для обработки ошибок.
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	FREE_IMAGE_FORMAT fif = 0;

	switch( IsSupTexture(texturePath) ){
		case NOT_SUPPORTED:
			fprintf(stderr, "Error: image format isn't supported yet.\n");
			return 0;
			break;
		case SUP_BMP:
			fif = FIF_BMP;
			break;
		case SUP_JPG:
			fif = FIF_JPEG;
			break;
	}

	FIBITMAP* bitmap = FreeImage_Load(fif, texturePath, 0);

	if( bitmap == 0 ){
		fprintf(stderr, "Error: image \"%s\" isn't loaded.\n", texturePath);
		return 0;
	}else
		return bitmap;
}

TextureInfo CreateGLTexture(FIBITMAP* bitmap){
	TextureInfo texInfo = {0,0,0};
	if( bitmap == 0 )
		return texInfo;

	GLint width  = FreeImage_GetWidth(bitmap);
	GLint height = FreeImage_GetHeight(bitmap);

	// Pointer to texture colors.
	GLubyte* imgData = (GLubyte*)FreeImage_GetBits(bitmap);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// For bmp, jpg.

	// Compressed image. Less memory but more time to load.
	/*glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imgData);*/

	// Uncompressed image. More memory but less time to load.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imgData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	FreeImage_Unload(bitmap);
	glBindTexture(GL_TEXTURE_2D, 0);

	texInfo.textureID = texture;
	texInfo.width = width;
	texInfo.height = height;
	return texInfo;
}

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char* message){
	printf("\n*** ");
	if( fif != FIF_UNKNOWN )
		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
	printf(message);
	printf(" ***\n");
}

GLdouble GetTime(){
	struct timeval t;
	gettimeofday(&t, NULL);
	return (t.tv_sec + t.tv_usec/1000000.0);
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
	winInfo.win = CreateWindow(winInfo.width, winInfo.height, winName);
	if( winInfo.win == 0 ){
		fprintf(stderr, "Failed to create window.\n");
		return 0;
	}

	// Создаём контекст OpenGL.
	winInfo.GLContext = CreateOpenGLContext(glversion_major, glversion_minor);
	if( winInfo.GLContext == 0 ){
		fprintf(stderr, "Failed to create OpenGL context.\n");
		return 0;
	}

	// Делаем контекст текущим.
	printf("Making context current.\n");
	glXMakeCurrent( winInfo.display, winInfo.win, winInfo.GLContext );

	// Создаём vertex array object и буфер для вершин.
	CreateVertexArray();

	// Создаём шейдер.
	shader = CreateShaderStr(ver_shader, frag_shader);
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
	zOffsetID	  = glGetUniformLocation(shader, "zOffset");
	yOffsetID	  = glGetUniformLocation(shader, "yOffset");

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
	glUniform1f(zOffsetID, zOffset);
	glUniform1f(yOffsetID, yOffset);

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
	GLdouble start, end;

	switch( xev.type ){
		case Expose :
			XGetWindowAttributes(winInfo.display, winInfo.win, &wa);
			glViewport(0,0, wa.width, wa.height);
			winInfo.width = wa.width;
			winInfo.height = wa.height;
			winInfo.ratio = wa.width/(float)wa.height;
			Mat4x4Ortho(&orthoMat, winInfo.ratio*imageScale, 1*imageScale, 0.1f, 10.0f);
			XStoreName( winInfo.display, winInfo.win, gTexInfo.name );
			XMapWindow( winInfo.display, winInfo.win );
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
			
		case KeyRelease :
			switch( XLookupKeysym(&xev.xkey, 0) ){
				case XK_Escape :
					winInfo.isRunning = GL_FALSE;
					break;

				case XK_Right :
					start = GetTime();

					ResetCamera();
					glDeleteTextures(1, &gTexInfo.textureID);
					gTexInfo = GetNextImage();

					end = GetTime();
					// Время на загрузку текстуры.
					printf("%.2f s.\n", end - start);

					// Анимация перелистывания.
					if( isAnimationEnable )
						AnimateImageShow(GL_TRUE);
					zOffset = yOffset = 0.0f;
					Render();
					XStoreName( winInfo.display, winInfo.win, gTexInfo.name );
					XMapWindow( winInfo.display, winInfo.win );
					break;

				case XK_Left :
					ResetCamera();
					glDeleteTextures(1, &gTexInfo.textureID);
					gTexInfo = GetPrevImage();

					// Анимация перелистывания.
					if( isAnimationEnable )
						AnimateImageShow(GL_FALSE);
					zOffset = yOffset = 0.0f;
					Render();
					XStoreName( winInfo.display, winInfo.win, gTexInfo.name );
					XMapWindow( winInfo.display, winInfo.win );
					break;
			}
			break;
	};
}

void CreateVertexArray(){
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

void AnimateImageShow(GLboolean next){
	GLdouble animStart = 0.0, 
			 animLen = 0.3, 
			 animProgress = 0.0;
	GLdouble coef = -1/8.0;
	animStart = GetTime();
	animProgress = GetTime() - animStart;

	if( next ){
		while( animProgress < animLen ){
			zOffset = (GetTime() - animStart)*winInfo.ratio/animLen - winInfo.ratio;
			yOffset = coef*zOffset*zOffset;
			Render();
			animProgress = GetTime() - animStart;
		}
	}else {
		while( animProgress < animLen ){
			zOffset = (GetTime() - animStart)*winInfo.ratio/animLen - winInfo.ratio;
			zOffset = -zOffset;
			yOffset = coef*zOffset*zOffset;
			Render();
			animProgress = GetTime() - animStart;
		}
	}
}
