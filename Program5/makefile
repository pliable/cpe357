CC = gcc

CFLAGS = -Wall -pedantic -g

LD = gcc

LDFLAGS = -Wall -g

#all: hencode hdecode

parseline: parseline.o
	$(LD) $(LDFLAGS) -o parseline parseline.o

parseline.o: parseline.c
	$(CC) $(CFLAGS) -c parseline.c
