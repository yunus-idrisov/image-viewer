
# -D GL_GLEXT_PROTOTYPES это эквивалент #define GL_GLEXT_PROTOTYPES 1
# Это определение необходимо, чтобы включить объявление некоторых
# функций OpenGL(например, glGenVertexArrays(...))

CFLAGS := -D GL_GLEXT_PROTOTYPES -pthread -std=c99 2>log
LIBS   := -lX11 -lGL -lfreeimage

ImageViewer : main.o math.o strlist.o directorywalk.o globalfuns.o
	gcc $^ -o $@ $(LIBS) $(CFLAGS)

main.o : main.c GlobalFuns.h GlobalVars.h
	gcc -c $< -o $@ $(CFLAGS)

math.o : Math.c Math.h
	gcc -c $< -o $@ $(CFLAGS)

strlist.o : StrList.c StrList.h
	gcc -c $< -o $@ $(CFLAGS)

directorywalk.o : DirectoryWalk.c DirectoryWalk.h GlobalFuns.h
	gcc -c $< -o $@ $(CFLAGS)

globalfuns.o : GlobalFuns.c GlobalFuns.h
	gcc -c $< -o $@ $(CFLAGS)

.PHONY : clean
clean : 
	rm -f ImageViewer main.o math.o strlist.o directorywalk.o globalfuns.o
