CC = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all clean run

all: todo

todo: main.c
	$(CC) $(CFLAGS) main.c -o todo

run: todo
	./todo list

clean:
	rm -f todo
	