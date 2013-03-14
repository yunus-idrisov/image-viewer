#ifndef DIRECTORYWALK_H
#define DIRECTORYWALK_H
#include "GlobalTypes.h"
#include "GlobalFuns.h"

/*
 Функции этого заголовочного файла обеспечивают
 просмотр папки в поисках фотографий.
 Сначала должна быть вызвана функция OpenWalkDir(...),
 чтобы указать папку, из которой берутся изображения.
 Потом с помощью GetNextImage() и GetPrevImage()
 возвращается следующее или предыдущее изображение.
 Если изображение не может быть прочитано или 
 папка вообще не содержит изображений, то поле 
 textureID структуры TextureInfo устанавливается равным 0.
 По завершении просмотра папки должна быть вызвана
 функция CloseWalkDir() для освобождения ресурсов.
 В любой момент времени может быть открыта только
 одна папка для просмотра.
*/

int OpenWalkDir(const char* path);// При ошибке возвращается -1.
TextureInfo GetNextImage();
TextureInfo GetPrevImage();
int CloseWalkDir();// При ошибке возвращается -1.

#endif // DIRECTORYWALK_H
