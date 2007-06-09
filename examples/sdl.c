#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <SDL_ttf.h>

#define DEFAULT_WIDTH  1360
#define DEFAULT_HEIGHT 760
#define DEFAULT_DEPTH  24

static SDL_Surface *
image_load (char *filename, int w, int h)
{
  SDL_Surface *img, *img2;

  img = IMG_Load (filename);
  if (!img)
    return NULL;
  printf ("Loaded a %d x %d image\n", img->w, img->h);

  if (w < 0 || h < 0)
    return img;

  /* scaling */
  img2 = zoomSurface (img, (float) w / img->w, (float) h / img->h, 1);
  if (!img2)
    return img;
  printf ("Scaled to a %d x %d image\n", img2->w, img2->h);

  return img2;
}

static TTF_Font *
font_load (char *filename, int size, int style)
{
  TTF_Font *font;
  
  if (!TTF_WasInit ())
    TTF_Init ();
  
  font = TTF_OpenFont (filename, 24);
  if (!font)
    return NULL;
  
  TTF_SetFontStyle (font, style);

  return font;
}

static SDL_Surface *
text_create (TTF_Font *font, char *str, int r, int g, int b)
{
  SDL_Surface *txt;
  SDL_Color color = {r, g, b, 255};

  txt = TTF_RenderUTF8_Blended (font, str, color);
  if (!txt)
  {
    printf (SDL_GetError());
    return NULL;
  }
  
  return txt;
}

static void
surface_blit (SDL_Surface *screen, SDL_Surface *s, int x, int y)
{
  SDL_Rect src, dest;

  src.x = 0;
  src.y = 0;
  src.w = s->w;
  src.h = s->h;

  dest = src;
  dest.x = x;
  dest.y = y;
  SDL_BlitSurface (s, &src, screen, &dest);
  SDL_UpdateRect (screen, dest.x, dest.y, dest.w, dest.h);
  SDL_Flip (screen);
}

int
main (int argc, char **argv)
{
  const SDL_VideoInfo *vi;
  char vo_driver[128];
  int flags = SDL_SWSURFACE;
  SDL_Rect **modes;
  SDL_Surface *screen;
  SDL_Event event;
  Uint32 bpp;
  SDL_Surface *bg;
  SDL_Surface *logo;
  TTF_Font *font;
  SDL_Surface *txt;
  int posx = 300, posy = 500;
  
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
    printf ("HW Surfaces enabled\n");
    flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
  }

  if (vi->wm_available)
    printf ("Windows Manager available\n");

  printf ("Video Memory: %d kB\n", vi->video_mem);
  printf ("Resolution: %d x %d\n", vi->current_w, vi->current_h);

  modes = SDL_ListModes (NULL, flags);

  /* Check if there are any modes available */
  if (modes == (SDL_Rect **) 0)
  {
    printf ("No modes available!\n");
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
    SDL_WM_SetCaption ("SDL Test App", NULL);

  /* load a background image */
  bg = image_load ("back_main.png", DEFAULT_WIDTH, DEFAULT_HEIGHT);
  surface_blit (screen, bg, 0, 0);

  /* load a logo image */
  logo = image_load ("logo.png", 200, -1);
  surface_blit (screen, logo, 200, 50);

  /* load a font */
  font = font_load ("FreeSans.ttf", 24, TTF_STYLE_NORMAL);

  /* display text */
  txt = text_create (font, "SDL UTF-8 TTF test", 255, 255, 0);
  surface_blit (screen, txt, posx, posy);
  
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
      {
        printf ("Quit\n");
        goto sdl_quit;
      }
      else if (keysym.sym == SDLK_UP)
      {
        posy -= 3;
        surface_blit (screen, txt, posx, posy);
      }
      else if (keysym.sym == SDLK_DOWN)
      {
        posy += 3;
        surface_blit (screen, txt, posx, posy);
      }
      else if (keysym.sym == SDLK_LEFT)
      {
        posx -= 3;
        surface_blit (screen, txt, posx, posy);
      }
      else if (keysym.sym == SDLK_RIGHT)
      {
        posx += 3;
        surface_blit (screen, txt, posx, posy);
      }
      break;

    case SDL_QUIT:
      printf ("Quit\n");
      goto sdl_quit;
    }
  }

 sdl_quit:
  SDL_FreeSurface (bg);
  TTF_CloseFont (font);
  TTF_Quit ();
  SDL_Quit ();
  return 0;
}
