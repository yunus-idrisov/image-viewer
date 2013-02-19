#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeImage.h>
#include "HelperFuns.h"
#include "icon_bitmap"

#define MAX_SHADER_SIZE		8192
#define MAX_LINE_SIZE		128

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

Display* 	display = NULL;
static GLXFBConfig bestFbc;

Window createWindow(GLuint width, GLuint height, const char* title){
	Window win;

	if( (display = XOpenDisplay(NULL)) == NULL ){
		fprintf(stderr, "Cannot connect to X server %s\n", XDisplayName(NULL) );
		return 0;
	}

	int defScreen = XDefaultScreen( display );

	// Извлечение версии GLX.
	int glx_major, glx_minor;
	if( !glXQueryVersion(display, &glx_major, &glx_minor) ){
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
	GLXFBConfig* fbc = glXChooseFBConfig(display, defScreen, visual_attributes, &fbcount);
	if( fbc == NULL ){
		printf("Failed to retrieve a FB configs.\n");
		return 0;
	}
	printf("Found %d matching FB configs.\n", fbcount);

	// Выбираем  FB с наибольшим количеством сэмплов.
	printf("Getting XVisualInfos:\n");
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for(int i = 0; i < fbcount; i++){
		XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
		if( vi ){
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES, &samples);
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
	XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
	printf("Chosen visual ID = 0x%x\n", vi->visualid);

	// Создаём colormap.
	printf("Creating colormap.\n");
	Colormap cmap;
	cmap = XCreateColormap( display, 
							RootWindow(display, vi->screen),
							vi->visual,
							AllocNone );

	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

	printf("Creating window.\n");
	win = XCreateWindow(display, RootWindow(display, vi->screen),
							   0, 0, width, height, 0, vi->depth, InputOutput,
							   vi->visual,
							   CWBorderPixel | CWColormap | CWEventMask, &swa);

	XFree(vi);
	XFreeColormap( display, cmap );
	if( win == NULL ){
		fprintf(stderr, "Failed to create window.\n");
		return 0;
	}

	XStoreName(display, win, title);
	printf("Mapping window.\n");
	XMapWindow( display, win );

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

	icon_pixmap = XCreateBitmapFromData(display, win,
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
	XSetWMProperties( display, win, &windowName, &iconName, NULL, 0, size_hints, wm_hints, class_hints);

	return win;
}

static int ctxErrorOccured = 0;
int ctxErrorHandler(Display* dpy, XErrorEvent* ev){
	ctxErrorOccured = 1;
	return 0;
}

GLXContext createOpenGLContext(int ver_major, int ver_minor){
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
	glctx = glXCreateContextAttribsARB(display, bestFbc, 0, True, context_attribs);

	XSync( display, False );
	if( !ctxErrorOccured && glctx ){
		printf("OpenGL %d.%d context created.\n", ver_major, ver_minor);
	}
	else{
		printf("Cannot create OpenGL context.\n");
		XSetErrorHandler( oldHandler );
		return glctx;
	}

	XSync( display, False );

	// Восстанавливаем начальный обработчик ошибок Xlib.
	XSetErrorHandler( oldHandler );
	if( ctxErrorOccured || !glctx ){
		printf("Cannot create OpenGL context.\n");
		return glctx;
	}

	// Проверяем является созданный контекст direct контекстом.
	if( !glXIsDirect( display, glctx ) )
		printf("Indirect GLX rendering context obtained\n");
	else
		printf("Direct GLX rendering context obtained.\n");

	return glctx;
}

GLuint CreateProgram(const char *vertex_shader_path, const char *fragment_shader_path){
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
	GLuint program = glCreateProgram();
	glAttachShader(program, verShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	// Проверём не возникли ли ошибки во время компоновки.
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if( result == GL_FALSE ){
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *errMessage = (GLchar*)malloc(infoLogLength + 1);
		glGetProgramInfoLog(program, infoLogLength, NULL, errMessage);
		errMessage[infoLogLength] = '\0';
		fprintf(stderr, "Error in shader program: %s\n", errMessage);
		free(errMessage);
		return 0;
	}

	glDeleteShader(verShader);
	glDeleteShader(fragShader);

	return program;
}

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char* message);

int isSupTexture(const char* textureName){
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

TextureInfo loadTexture(const char *texturePath){
	// Функция для обработки ошибок.
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	TextureInfo texInfo = {0,0,0};

	FREE_IMAGE_FORMAT fif = 0;

	switch( isSupTexture(texturePath) ){
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

FIBITMAP* getFIBITMAP(const char *texturePath){
	// Функция для обработки ошибок.
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	FREE_IMAGE_FORMAT fif = 0;

	switch( isSupTexture(texturePath) ){
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

TextureInfo createGLTexture(FIBITMAP* bitmap){
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
