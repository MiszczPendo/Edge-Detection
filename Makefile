all:
	gcc -I src/include -L src/lib main.c -o Edge_Detection -lmingw32 -lcomdlg32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf