#include <stdio.h>
#include "GlobalVars.h"
#include "GlobalFuns.h"

int main(int argc, char *argv[]){
	char* path = CmdLineParse(argc, argv);
	if( path == NULL )
		return 0;

	if( InitAppliction("Image viewer", winInfo.width, winInfo.height, 3, 3) == 0 ){
		fprintf(stderr, "Initialization failed.\n");
		return 0;
	}

	if( OpenWalkDir( path ) == -1 )
		return 0;
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
