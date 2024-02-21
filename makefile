CC = gcc
CFLAGS = -Wall -Wextra -pthread

s-talk: s-talk.c list.c
	$(CC) $(CFLAGS) s-talk.c -o s-talk
	
clean:
	rm -f s-talk
