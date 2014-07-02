CC = gcc

CFLAGS = -Wall -Wextra -pedantic -ansi -g

LD = gcc

LDFLAGS = -Wall -Wextra -g

all: hencode hdecode

hencode: hencode.o
	$(LD) $(LDFLAGS) -o hencode hencode.o

hencode.o: hencode.c
	$(CC) $(CFLAGS) -c hencode.c

hdecode: hdecode.o
	$(LD) $(LDFLAGS) -o hdecode hdecode.o

hdecode.o: hdecode.c
	$(CC) $(CFLAGS) -c hdecode.c
