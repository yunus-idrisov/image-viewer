
# -D GL_GLEXT_PROTOTYPES это эквивалент #define GL_GLEXT_PROTOTYPES 1
# Это определение необходимо, чтобы включить объявление некоторых
# функций OpenGL(например, glGenVertexArrays(...))

CFLAGS := -D GL_GLEXT_PROTOTYPES -std=c99 2>log

main.out : main.o math.o helperfuns.o strlist.o directorywalk.o
	gcc $^ -o $@ -lGL -lfreeimage $(CFLAGS)

main.o : main.c
	gcc -c $< -o $@ $(CFLAGS)

helperfuns.o : HelperFuns.c HelperFuns.h
	gcc -c $< -o $@ $(CFLAGS)

math.o : Math.c Math.h
	gcc -c $< -o $@ $(CFLAGS)

strlist.o : StrList.c StrList.h
	gcc -c $< -o $@ $(CFLAGS)

directorywalk.o : DirectoryWalk.c DirectoryWalk.h
	gcc -c $< -o $@ $(CFLAGS)
