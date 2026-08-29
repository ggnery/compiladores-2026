CC = gcc-14
simpleLang:  simpleLang.o lexico.o
	$(CC) simpleLang.o lexico.o -o simpleLang
simpleLang.o: simpleLang.c
	$(CC) -c simpleLang.c -o simpleLang.o
simpleLang.c:  simpleLang.y
	bison -d -osimpleLang.c simpleLang.y
lexico.o: lexico.c
	$(CC) -c lexico.c -o lexico.o
lexico.c: lexico.l
	flex -olexico.c  lexico.l
clean: 
	rm *.o  simpleLang *.c
