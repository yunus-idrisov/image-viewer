#include <stdio.h>
#include "GlobalVars.h"
#include "GlobalFuns.h"

int main(int argc, char *argv[]){
	if( InitAppliction("Image viewer", winInfo.width, winInfo.height, 3, 3) == 0 ){
		fprintf(stderr, "Initialization failed.\n");
		return 0;
	}

	/*
	if( argc == 2 ){
		if( OpenWalkDir( argv[1] ) == -1 ){
			fprintf(stderr, "Error while opening path \"%s\"\n", argv[1]);
			return 1;
		}
	}else {
		printf("Please, specify path to directory with images.\n");
		return 1;
	}
	*/

	/*OpenWalkDir("/home/yunus/Pictures/");*/
	OpenWalkDir("./images");
	/*OpenWalkDir("/home/yunus/Desktop/100CANON");*/
	/*OpenWalkDir("/media/Disc_D/Copy_E/Photo/National Geographic/National Geographic 2011");*/
	gTexInfo = GetNextImage();

	XEvent xev;
	// Основной цикл приложения.
	while( winInfo.isRunning ){
		XNextEvent(winInfo.display, &xev);
		EventHandler( xev );
	}

	CloseWalkDir();
	CleanUp();
	return 0;
}
