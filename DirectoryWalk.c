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

static DIR* 		dir = 0;
static StrList* 	imageNames = 0;
static ListNode*	curImagePointer = 0;
static int			imagePathLen = 0;
static char			imagePath[NAME_MAX];

/*
 * Нулевой элемент массива хранит предыдущее
 * изображение, 1 - текущее, 2 - следующее.
 */
static TextureInfo  curImage = {0,0,0};
static pthread_t 	thread_id1 = 0;
static pthread_t 	thread_id2 = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static FIBITMAP*	pNextFIBITMAP = 0;
static FIBITMAP*	pPrevFIBITMAP = 0;
/*
 * Какое изображение должно быть загружено функцией thread_func(...).
 * 0 означает предыдущее, 2 - следующее.
 */
static int			imageToLoad;

ListNode*	getNextNodeCycle(ListNode* curNode);
ListNode*	getPrevNodeCycle(ListNode* curNode);

static void* thread_func(void* arg){
	pthread_mutex_lock( &mtx );
	switch( imageToLoad ){
		case 0:
			if( curImagePointer != 0 ){
				FreeImage_Unload( pNextFIBITMAP );
				pNextFIBITMAP = 0;

				strcat(imagePath, curImagePointer->str);
				pNextFIBITMAP = getFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';

				curImagePointer = getPrevNodeCycle( curImagePointer );

				ListNode* prevImagePointer = getPrevNodeCycle( curImagePointer );
				strcat(imagePath, prevImagePointer->str);
				pPrevFIBITMAP = getFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';
			}
			break;
		case 2:
			if( curImagePointer != 0 ){
				FreeImage_Unload( pPrevFIBITMAP );
				pPrevFIBITMAP = 0;

				strcat(imagePath, curImagePointer->str);
				pPrevFIBITMAP = getFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';

				curImagePointer = getNextNodeCycle( curImagePointer );

				ListNode* nextImagePointer = getNextNodeCycle( curImagePointer );
				strcat(imagePath, nextImagePointer->str);
				pNextFIBITMAP = getFIBITMAP( imagePath );
				imagePath[imagePathLen] = '\0';
			}
			break;
	}
	pthread_mutex_unlock( &mtx );
	return 0;
}

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
	// Загрузка первого изображения.
	if( curImagePointer != 0 ){ // Если список изображений не пуст.
		strcat(imagePath, curImagePointer->str);
		curImage = loadTexture( imagePath );
		imagePath[imagePathLen] = '\0';

		ListNode* prevImagePointer = getPrevNodeCycle( curImagePointer );
		strcat(imagePath, prevImagePointer->str);
		pPrevFIBITMAP = getFIBITMAP( imagePath );
		imagePath[imagePathLen] = '\0';

		ListNode* nextImagePointer = getNextNodeCycle( curImagePointer );
		strcat(imagePath, nextImagePointer->str);
		pNextFIBITMAP = getFIBITMAP( imagePath );
		imagePath[imagePathLen] = '\0';
	}

	/*ShowStrList(imageNames);*/
	return 1;
}

TextureInfo getNextImage(){
	pthread_mutex_lock( &mtx );
	TextureInfo texInfo = {0,0,0};

	void* status;
	pthread_join(thread_id1, &status);
	texInfo = createGLTexture( pNextFIBITMAP );

	imageToLoad = 2;
	if( pthread_create(&thread_id1, NULL, thread_func, NULL) != 0 )
		fprintf(stderr, "create error\n");

	pthread_mutex_unlock( &mtx );
	return texInfo;
}

TextureInfo getPrevImage(){
	pthread_mutex_lock( &mtx );
	TextureInfo texInfo = {0,0,0};

	void* status;
	pthread_join(thread_id2, &status);
	texInfo = createGLTexture( pPrevFIBITMAP );

	imageToLoad = 0;
	if( pthread_create(&thread_id2, NULL, thread_func, NULL) != 0 )
		fprintf(stderr, "create error\n");

	pthread_mutex_unlock( &mtx );
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

ListNode* getNextNodeCycle(ListNode* curImageNode){
	if( curImageNode != 0 ){
		curImageNode = curImageNode->next;
		if( curImageNode == 0 )
			curImageNode = imageNames->head;
		return curImageNode;
	}else
		return 0;
}

ListNode* getPrevNodeCycle(ListNode* curImageNode){
	if( curImageNode != 0 ){
		curImageNode = curImageNode->prev;
		if( curImageNode == 0 )
			curImageNode = imageNames->tail;
		return curImageNode;
	}else
		return 0;
}
