/* GeeXboX Open Media Center.
 * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <SDL/SDL.h>

#include "widgets/widget.h"

#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720
#define DEFAULT_DEPTH  24
#define DEFAULT_WM_CAPTION "GeeXboX Open Media Center"

SDL_Surface *screen;

int
main (int argc, char **argv)
{
  const SDL_VideoInfo *vi;
  char vo_driver[128];
  int flags = SDL_SWSURFACE;
  SDL_Rect **modes;
  SDL_Event event;
  Uint32 bpp;

  widget_t *bg = NULL;
  
  if (SDL_Init (SDL_INIT_VIDEO) < 0)
  {
    fprintf (stderr, "Unable to init SDL: %s\n", SDL_GetError ());
    goto sdl_quit;
  }

  SDL_VideoDriverName (vo_driver, 128);
  printf ("Using Video Driver: %s\n", vo_driver);
  
  vi = SDL_GetVideoInfo ();
  
  if (vi->hw_available)
  {
    fprintf (stderr, "HW Surfaces enabled\n");
    flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
  }

  printf ("Resolution: %d x %d\n", vi->current_w, vi->current_h);

  modes = SDL_ListModes (NULL, flags);

  /* Check if there are any modes available */
  if (modes == (SDL_Rect **) 0)
  {
    fprintf (stderr, "No modes available!\n");
    goto sdl_quit;
  }

  /* Check if our resolution is restricted */
  if (modes == (SDL_Rect **) -1)
    printf ("All resolutions available.\n");
  else
  {
    int i;
    printf ("Available Modes\n");
    for (i = 0; modes[i]; i++)
      printf ("  %d x %d\n", modes[i]->w, modes[i]->h);
  }

  printf ("Checking mode %dx%d@%d\n",
          DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_DEPTH);
  bpp = SDL_VideoModeOK (DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_DEPTH, flags);

  if (!bpp)
  {
    printf ("Mode not available.\n");
    goto sdl_quit;
  }

  printf ("SDL Recommends %dx%d@%d\n", DEFAULT_WIDTH, DEFAULT_HEIGHT, bpp);
  screen = SDL_SetVideoMode (DEFAULT_WIDTH, DEFAULT_HEIGHT, bpp, flags);

  if (vi->wm_available)
    SDL_WM_SetCaption (DEFAULT_WM_CAPTION, NULL);

  bg = image_new ("background", 0, 1, 1, "data/background.png", NULL,
                  0, 0, -1, -1);
  widget_show (bg);

  bg = image_new ("banner-top", 0, 1, 1, "data/banner-top.png", NULL,
                  0, 0, -1, -1);
  widget_show (bg);

  SDL_Flip (screen);
  
  /* events handling */
  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while (SDL_WaitEvent (&event) != 0)
  {
    SDL_keysym keysym;

    switch (event.type)
    {

    case SDL_KEYDOWN : // Keyboard Events
      keysym = event.key.keysym;
      if (keysym.sym == SDLK_q)
        goto sdl_quit;
      break;
      
    case SDL_QUIT:
      goto sdl_quit;
    }
  }

 sdl_quit:
  widget_free (bg);
  SDL_Quit ();
  return 0;
}
