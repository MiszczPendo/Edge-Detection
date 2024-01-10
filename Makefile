all:
	gcc -I src/include -L src/lib main.c -o main -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lcomdlg32 -lSDL2_ttf