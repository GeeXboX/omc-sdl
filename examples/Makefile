CC = gcc
CFLAGS = -Wall -g

crawler: crawler.c
	$(CC) $< $(CFLAGS) -lavformat -lavcodec -lswscale -lavutil -o $@

all: crawler

clean:
	rm -f crawler
