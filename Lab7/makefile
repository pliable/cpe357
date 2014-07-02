CC = gcc

CFLAGS = -Wall -Wextra -pedantic -g

LD = gcc

LDFLAGS = -Wall -Wextra -g

forkit: forkit.o
	$(LD) $(LDFLAGS) -o forkit forkit.o

forkit.o: forkit.c
	$(CC) $(CFLAGS) -c forkit.c
