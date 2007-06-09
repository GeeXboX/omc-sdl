CC = gcc
CFLAGS = -Wall -g

SDL_CFLAGS=`sdl-config --cflags`
SDL_LIBS=`sdl-config --libs` -lSDL_image -lSDL_gfx -lSDL_ttf

all: crawler sdl

crawler: crawler.c
	$(CC) $< $(CFLAGS) -lavformat -lavcodec -lswscale -lavutil -o $@

sdl: sdl.c
	$(CC) $< $(CFLAGS) $(SDL_CFLAGS) $(SDL_LIBS) -o $@

clean:
	rm -f crawler sdl
