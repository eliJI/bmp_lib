cls
gcc -c bittype.c %c89%
gcc -c bmp.c  %c89%
gcc bittype.o bmp.o main.c %c89%