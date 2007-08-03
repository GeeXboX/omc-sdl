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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL_timer.h>

#include "event.h"
#include "screen.h"
#include "widgets/widget.h"
#include "omc.h"

SDL_TimerID clock_timer;

static Uint32
clock_cb (Uint32 interval, void *param)
{
  widget_t *clock = (widget_t *) param;
  time_t tt = time (NULL);
  char *t = ctime (&tt);
  text_set_str (clock, t);

  return interval;
}

static int
screen_main_event_handler (screen_t *screen, SDL_Event *ev)
{
  SDL_keysym keysym;

  switch (ev->type)
  {
  case SDL_KEYDOWN : /* Keyboard Events */
    keysym = ev->key.keysym;
    if (keysym.sym == SDLK_a)
    {
      widget_t *w;
      int state;
      
      printf ("Stroke the A key\n");

      w = widget_get_by_id (screen->wlist, "playdvd-caption");
      state = widget_get_flag (w, WIDGET_FLAG_FOCUSED);
      widget_set_focus (w, state ? 0 : 1);
      
      return 0;
    }
  }

  /* fall back to generic event handler */
  return default_event_handler (ev);
}

static void
screen_main_uninit (screen_t *screen)
{
  if (clock_timer)
    SDL_RemoveTimer (clock_timer);
}

void
screen_main_init (screen_t *screen)
{
  widget_t *bg = NULL;
  widget_t *bt = NULL;
  widget_t *bb = NULL;
  widget_t *frame = NULL;
  widget_t *pics = NULL;
  widget_t *txt1 = NULL;
  widget_t *txt2 = NULL;
  widget_t *clock = NULL;
  time_t tt = time (NULL);
  char *t = ctime (&tt);
    
  if (!screen)
    return;

  screen->handle_event = screen_main_event_handler;
  screen->uninit = screen_main_uninit;

  /* populate screen */
  bg = image_new ("background", NULL, 0, 1, 1, "data/background.png", NULL,
                  0, 0, 0, 0, NULL, NULL, "100%", "100%");
  screen_add_widget (screen, bg);

  bt = image_new ("banner-top", bg, 0, 1, 1, "data/banner-top.png", NULL,
                  0, 0, 0, 0, NULL, NULL, "100%", NULL);
  screen_add_widget (screen, bt);

  bb = image_new ("banner-bottom", bg, 0, 1, 1, "data/banner-bottom.png", NULL,
                  0, omc->h - 145, 0, 0, NULL, NULL, NULL, NULL);
  screen_add_widget (screen, bb);

  frame = image_new ("frame", bg, 0, 1, 1, "data/frame.png", NULL,
                     30, 220, 500, 380, NULL, NULL, NULL, NULL);
  screen_add_widget (screen, frame);

  pics = image_new ("menu-pics", bg, 0, 1, 1, "data/image.png", NULL,
                    omc->w - 600, 135, 600, 480, NULL, NULL, NULL, NULL);
  screen_add_widget (screen, pics);

  txt1 = text_new ("playdvd-caption", frame, 1, 1, 2, "Play DVD",
                   "examples/FreeSans.ttf", 24,
                   0x33, 0x85, 0xF4, 0x62, 0x23, 0x4E, 300, 300, 0, 0,
                   NULL, NULL, NULL, NULL);
  screen_add_widget (screen, txt1);

  txt2 = text_new ("watchtv-caption", frame, 1, 1, 2, "Watch TV and have fun!",
                   "examples/FreeSans.ttf", 24,
                   0x33, 0x85, 0xF4, 0x62, 0x23, 0x4E, 300, 350, 0, 0,
                   NULL, NULL, NULL, NULL);
  screen_add_widget (screen, txt2);

  widget_set_neighbour (txt1, txt2, NEIGHBOURS_DOWN);
  widget_set_neighbour (txt2, txt1, NEIGHBOURS_UP);
  
  clock = text_new ("clock", bt, 0, 1, 2, t,
                   "examples/FreeSans.ttf", 24,
                    0xFF, 0xFF, 0xFF, 0, 0, 0, 990, 85, 0, 0,
                    NULL, NULL, NULL, NULL);
  clock_cb (10, clock);
  screen_add_widget (screen, clock);

  clock_timer = SDL_AddTimer (1000, clock_cb, clock);
}
