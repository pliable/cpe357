CC = gcc

CFLAGS = -Wall -Wextra -pedantic -g

LD = gcc

LDFLAGS = -Wall -Wextra -g

all: mytar

mytar: mytar.o
	$(LD) $(LDFLAGS) -o mytar mytar.o

mytar.o: mytar.c
	$(CC) $(CFLAGS) -c mytar.c
