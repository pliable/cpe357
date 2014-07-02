CC = gcc

CFLAGS = -Wall -Wextra -pedantic -g

LD = gcc

LDFLAGS = -Wall -Wextra -pedantic -g

mush: mush.o
	$(LD) $(LDFLAGS) -o mush mush.o

mush.o: mush.c
	$(CC) $(CFLAGS) -c mush.c
