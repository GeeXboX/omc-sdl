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

#include "event.h"
#include "screen.h"
#include "widgets/widget.h"

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
      printf ("Stroke the A key\n");
      return 0;
    }
  }

  /* fall back to generic event handler */
  return default_event_handler (ev);
}

static void
screen_main_uninit (screen_t *screen)
{
  /* to be filled in */
}

void
screen_main_init (screen_t *screen)
{
  widget_t *bg = NULL;
  widget_t *banner = NULL;
  widget_t *txt1 = NULL;
  widget_t *txt2 = NULL;

  if (!screen)
    return;

  screen->handle_event = screen_main_event_handler;
  screen->uninit = screen_main_uninit;

  /* populate screen */
  bg = image_new ("background", 0, 1, 1, "data/background.png", NULL,
                  0, 0, -1, -1);
  screen_add_widget (screen, bg);

  banner = image_new ("banner-top", 0, 1, 1, "data/banner-top.png", NULL,
                      0, 0, -1, -1);
  screen_add_widget (screen, banner);

  txt1 = text_new ("playdvd-caption", 1, 1, 2, "Play DVD",
                   "examples/FreeSans.ttf", 24,
                   0x33, 0x85, 0xF4, 0x62, 0x23, 0x4E, 300, 300, -1, -1);
  screen_add_widget (screen, txt1);

  txt2 = text_new ("watchtv-caption", 0, 1, 2, "Watch TV",
                   "examples/FreeSans.ttf", 24,
                   0x33, 0x85, 0xF4, 0, 0, 0, 300, 350, -1, -1);
  screen_add_widget (screen, txt2);
}
