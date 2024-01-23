CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -ggdb

file: main.c
	$(CC) $(CFLAGS) -o file main.c

clean:
	rm -f file hello.txt.* foo.*
