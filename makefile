CC = gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	$(CC) $(CFLAGS) s-talk.c shutdown_manager.c list.c -o s-talk -lpthread
	
clean:
	rm -f s-talk
