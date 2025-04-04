CC = gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	$(CC) $(CFLAGS) s-talk.c shutdown_manager/shutdown_manager.c \
	list/list.c threads/input_thread.c threads/keyboard_thread.c \
	threads/output_thread.c threads/screen_thread.c socket.c -o s-talk -lpthread
	
clean:
	rm -f s-talk
