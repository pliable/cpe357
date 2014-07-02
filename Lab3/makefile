CC = gcc

CFLAGS = -Wall -pedantic -ansi -g

LD = gcc

LDFLAGS = -Wall -g

htable: htable.o
	$(LD) $(LDFLAGS) -o htable htable.o

htable.o: htable.c
	$(CC) $(CFLAGS) -c htable.c
