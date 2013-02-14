#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeImage.h>
#include "HelperFuns.h"

#define MAX_SHADER_SIZE		8192
#define MAX_LINE_SIZE		128

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
