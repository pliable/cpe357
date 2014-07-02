CC = gcc

CFLAGS = -Wall -ansi -pedantic -g

LD = gcc

LDFLAGS = -Wall -ansi -pedantic -g

myuniq: myuniq.o
	$(LD) $(CFLAGS) -o myuniq myuniq.o

myuniq.o: myuniq.c
	$(CC) $(CFLAGS) -c myuniq.c
