CC = gcc

CFLAGS = -Wall -ansi -pedantic -g

LD = gcc

LFLAGS = -Wall -g

fw: fw.o hashtable.o
	$(LD) $(LFLAGS) -o fw fw.o hashtable.o

object: fw.c hashtable.c
	$(CC) $(CFLAGS) -c fw.c hashtable.c
