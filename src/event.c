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

#include <SDL.h>

#include "omc.h"
#include "widgets/widget.h"

int
default_event_handler (SDL_Event *event)
{
  SDL_keysym keysym;

  switch (event->type)
  {
  case SDL_KEYDOWN : // Keyboard Events
    keysym = event->key.keysym;
    if (keysym.sym == SDLK_q)
      omc_uninit ();
    else if (keysym.sym == SDLK_UP)
      widget_move_focus (omc->scr->current, NEIGHBOURS_UP);
    else if (keysym.sym == SDLK_DOWN)
      widget_move_focus (omc->scr->current, NEIGHBOURS_DOWN);
    else if (keysym.sym == SDLK_LEFT)
      widget_move_focus (omc->scr->current, NEIGHBOURS_LEFT);
    else if (keysym.sym == SDLK_RIGHT)
      widget_move_focus (omc->scr->current, NEIGHBOURS_RIGHT);
    break;
      
  case SDL_QUIT:
    omc_uninit ();
  }

  return 0;
}
