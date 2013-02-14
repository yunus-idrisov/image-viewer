#define _BSD_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "DirectoryWalk.h"
#include "StrList.h"

static DIR* 		dir = 0;
static StrList* 	imageNames = 0;
static ListNode*	curImagePointer = 0;
static char			imagePath[NAME_MAX];
static int			imagePathLen = 0;

int openWalkDir(const char* path){
	initStrList(&imageNames);
	dir = opendir(path);
	if( dir == NULL ){
		fprintf(stderr, "Error in open path \"%s\": ", path);
		fprintf(stderr, "%s.\n", strerror(errno));
		deleteStrList(&imageNames);
		return -1;
	}

	// Если рабочей папкой является не текущая, то
	// необходимо указывать абсолютный путь к фотографиям
	// в функциях getNextImage() и getPrevImage().
	strcpy(imagePath, path);
	if( imagePath[strlen(imagePath)-1] != '/' )
		strcat(imagePath, "/");
	imagePathLen = strlen(imagePath);

	struct dirent* de;
	// Считываем имена всех поддерживаемых изображений
	// в список imageNames.
	while( (de = readdir(dir)) != NULL ){
		// Если прочитанный файл обычный файл и является
		// поддерживаемой текстурой, то добавить его в 
		// список изображений.
		if( (de->d_type == DT_REG) && isSupTexture(de->d_name) )
			addStringToStrList(imageNames, de->d_name);
	}

	curImagePointer = imageNames->head;

	/*ShowStrList(imageNames);*/
	return 1;
}

TextureInfo getNextImage(){
	TextureInfo texInfo = {0,0,0};
	// Если список изображений не пуст.
	if( curImagePointer != 0 ){
		curImagePointer = curImagePointer->next;
		// Если достигли конца, то перейти к началу.
		if( curImagePointer == 0 )
			curImagePointer = imageNames->head;
		strcat(imagePath, curImagePointer->str);
		texInfo = loadTexture( imagePath );
		imagePath[imagePathLen] = '\0';
	}
	return texInfo;
}

TextureInfo getPrevImage(){
	TextureInfo texInfo = {0,0,0};
	// Если список изображений не пуст.
	if( curImagePointer != 0 ){
		curImagePointer = curImagePointer->prev;
		// Если достигли начала, то перейти к концу.
		if( curImagePointer == 0 )
			curImagePointer = imageNames->tail;
		strcat(imagePath, curImagePointer->str);
		texInfo = loadTexture( imagePath );
		imagePath[imagePathLen] = '\0';
	}
	return texInfo;
}

int closeWalkDir(){
	deleteStrList(&imageNames);

	if( closedir(dir) != 0 ){
		fprintf(stderr, "closedir(...) error: ");
		fprintf(stderr, "%s.\n", strerror(errno));
		return -1;
	}
	return 1;
}
