#ifndef DIRECTORYWALK_H
#define DIRECTORYWALK_H
#include "HelperFuns.h"

/*
 Функции этого заголовочного файла обеспечивают
 просмотр папки в поисках фотографий.
 Сначала должна быть вызвана функция openWalkDir(...),
 чтобы указать папку, из которой берутся изображения.
 Потом с помощью getNextImage() и getPrevImage()
 возвращается следующее или предыдущее изображение.
 Если изображение не может быть прочитано или 
 папка вообще не содержит изображений, то поле 
 textureID структуры TextureInfo устанавливается равным 0.
 По завершении просмотра папки должна быть вызвана
 функция closeWalkDir() для освобожения ресурсов.
*/

int openWalkDir(const char* path);// При ошибке возвращается -1.
TextureInfo getNextImage();
TextureInfo getPrevImage();
int closeWalkDir();// При ошибке возвращается -1.

#endif // DIRECTORYWALK_H
