#define _BSD_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "DirectoryWalk.h"
#include "StrList.h"

extern int mode;

static DIR* 		dir = 0;
static StrList* 	imageNames = 0;
static ListNode*	curImagePointer = 0;
static int			imagePathLen = 0;
static char			imagePath[NAME_MAX];

static pthread_t 	thread_id = 0;
static FIBITMAP*	pNextFIBITMAP = 0;
static FIBITMAP*	pPrevFIBITMAP = 0;

/*
 * Какое изображение должно быть загружено функцией thread_func(...).
 * 0 означает предыдущее, 2 - следующее.
 */
static int			imageToLoad;

ListNode*	GetNextNodeCycle(ListNode* curNode);
ListNode*	GetPrevNodeCycle(ListNode* curNode);

static void* thread_func(void* arg){
	switch( imageToLoad ){
		case 0:
			if( curImagePointer != 0 ){
				FreeImage_Unload( pNextFIBITMAP );
				pNextFIBITMAP = 0;

				strcat(imagePath, curImagePointer->str);
				pNextFIBITMAP = GetFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';

				curImagePointer = GetPrevNodeCycle( curImagePointer );

				ListNode* prevImagePointer = GetPrevNodeCycle( curImagePointer );
				strcat(imagePath, prevImagePointer->str);
				pPrevFIBITMAP = GetFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';
			}
			break;
		case 2:
			if( curImagePointer != 0 ){
				FreeImage_Unload( pPrevFIBITMAP );
				pPrevFIBITMAP = 0;

				strcat(imagePath, curImagePointer->str);
				pPrevFIBITMAP = GetFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';

				curImagePointer = GetNextNodeCycle( curImagePointer );

				ListNode* nextImagePointer = GetNextNodeCycle( curImagePointer );
				strcat(imagePath, nextImagePointer->str);
				pNextFIBITMAP = GetFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';
			}
			break;
	}
	return 0;
}

int OpenWalkDir(const char* path){
	InitStrList(&imageNames);
	dir = opendir(path);
	if( dir == NULL ){
		fprintf(stderr, "Error in open path \"%s\": %s\n", path, strerror(errno));
		DeleteStrList(&imageNames);
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
	errno = 0;
	while( (de = readdir(dir)) != NULL ){
		// Если прочитанный файл обычный файл и является
		// поддерживаемой текстурой, то добавить его в 
		// список изображений.
		if( (de->d_type == DT_REG) && IsSupTexture(de->d_name) )
			AddStringToStrList(imageNames, de->d_name);
	}
	if( errno != 0 ){
		fprintf(stderr, "Read path \"%s\" error: %s\n", path, strerror(errno));
		DeleteStrList(&imageNames);
		return -1;
	}
	SortStrList(imageNames);

	curImagePointer = imageNames->head;

	// Указатель на предыдущий и следующий изображения
	// сохраняются только при параллельном режиме работы.
	if( curImagePointer != 0 && mode == 1 ){ // Если список изображений не пуст.

		ListNode* prevImagePointer = GetPrevNodeCycle( curImagePointer );
		strcat(imagePath, prevImagePointer->str);
		pPrevFIBITMAP = GetFIBITMAP( imagePath );
		imagePath[imagePathLen] = '\0';

		ListNode* nextImagePointer = GetNextNodeCycle( curImagePointer );
		strcat(imagePath, nextImagePointer->str);
		pNextFIBITMAP = GetFIBITMAP( imagePath );
		imagePath[imagePathLen] = '\0';
	}

	/*ShowStrList(imageNames);*/
	return 1;
}

TextureInfo GetNextImage(){
	TextureInfo texInfo = {0,0,0,0};
	void* status;

	switch( mode ){
		case 1:
			pthread_join(thread_id, &status);
			texInfo = CreateGLTexture( pNextFIBITMAP );

			imageToLoad = 2;
			if( pthread_create(&thread_id, NULL, thread_func, NULL) != 0 )
				fprintf(stderr, "Thread create error.\n");

			ListNode* next = GetNextNodeCycle(curImagePointer);
			if( next != 0 )
				texInfo.name = next->str;
			break;

		case 2 : case 3:
			curImagePointer = GetNextNodeCycle( curImagePointer );
			if( curImagePointer != 0 ){
				strcat(imagePath, curImagePointer->str);
				texInfo = LoadTexture( imagePath );
				imagePath[imagePathLen] = '\0';
				texInfo.name = curImagePointer->str; 
			}
			break;
	}
	return texInfo;
}

TextureInfo GetPrevImage(){
	TextureInfo texInfo = {0,0,0,0};
	void* status;

	switch( mode ){
		case 1:
			pthread_join(thread_id, &status);
			texInfo = CreateGLTexture( pPrevFIBITMAP );

			imageToLoad = 0;
			if( pthread_create(&thread_id, NULL, thread_func, NULL) != 0 )
				fprintf(stderr, "Thread create error.\n");

			ListNode* prev = GetPrevNodeCycle(curImagePointer);
			if( prev != 0 )
				texInfo.name = prev->str;
			break;

		case 2 : case 3:
			curImagePointer = GetPrevNodeCycle( curImagePointer );
			if( curImagePointer != 0 ){
				strcat(imagePath, curImagePointer->str);
				texInfo = LoadTexture( imagePath );
				imagePath[imagePathLen] = '\0';
				texInfo.name = curImagePointer->str; 
			}
			break;
	}
	return texInfo;
}

int CloseWalkDir(){
	if( mode == 1 ){
		void* status;
		pthread_join( thread_id, &status );
		DeleteStrList(&imageNames);
		if( closedir(dir) != 0 ){
			fprintf(stderr, "closedir(...) error: %s.\n", strerror(errno));
			return -1;
		}
		FreeImage_Unload( pPrevFIBITMAP );
		pPrevFIBITMAP = 0;
		FreeImage_Unload( pNextFIBITMAP );
		pNextFIBITMAP = 0;
	}
	else{
		DeleteStrList(&imageNames);
		if( closedir(dir) != 0 ){
			fprintf(stderr, "closedir(...) error: %s.\n", strerror(errno));
			return -1;
		}
	}
	return 1;
}

ListNode* GetNextNodeCycle(ListNode* curImageNode){
	if( curImageNode != 0 ){
		curImageNode = curImageNode->next;
		if( curImageNode == 0 )
			curImageNode = imageNames->head;
		return curImageNode;
	}else
		return 0;
}

ListNode* GetPrevNodeCycle(ListNode* curImageNode){
	if( curImageNode != 0 ){
		curImageNode = curImageNode->prev;
		if( curImageNode == 0 )
			curImageNode = imageNames->tail;
		return curImageNode;
	}else
		return 0;
}
