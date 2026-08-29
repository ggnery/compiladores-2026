CC = gcc-14
simpleLang:  sintatico.o lexico.o
	$(CC) sintatico.o lexico.o -o simpleLang
sintatico.o: sintatico.c
	$(CC) -c sintatico.c -o sintatico.o
sintatico.c:  sintatico.y
	bison -HsimpleLang.h -osintatico.c sintatico.y
lexico.o: lexico.c
	$(CC) -c lexico.c -o lexico.o
lexico.c: lexico.l
	flex -olexico.c  lexico.l
clean: 
	rm *.o  simpleLang *.c
