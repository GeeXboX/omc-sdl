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
#include <SDL_thread.h>

#include "omc.h"
#include "display.h"
#include "screens/screen.h"
#include "widgets/widget.h"

#define TICK_INTERVAL 100 /* (10 fps = 1000 / 100ms) */

static Uint32 next_time;

static Uint32
time_left (void)
{
  Uint32 now;

  now = SDL_GetTicks ();
  if (next_time <= now)
  {
    next_time = now;
    return 0;
  }

  return next_time - now;
}

int
surface_blit (widget_t *widget, SDL_Surface *srf, SDL_Rect offset)
{
  if (!widget || !srf)
    return -1;

  printf ("Blitting Surface on (%d x %d) to (%d x %d)\n",
          offset.x, offset.y, offset.x + offset.w, offset.y + offset.h);
  if (widget->redraw_area.x != 0)
    printf ("where we asked for (%d x %d) to (%d x %d) only\n",
            widget->redraw_area.x, widget->redraw_area.y,
            widget->redraw_area.x + widget->redraw_area.w,
            widget->redraw_area.y + widget->redraw_area.h);
 
  if (SDL_MUSTLOCK (omc->display))
    SDL_LockSurface (omc->display);

  if (widget->redraw_area.x != 0)
  {
    SDL_BlitSurface (srf, &(widget->redraw_area),
                     omc->display, &(widget->redraw_area));
    widget->redraw_area.x = 0;
    widget->redraw_area.y = 0;
    widget->redraw_area.w = 0;
    widget->redraw_area.h = 0;
  }
  else
    SDL_BlitSurface (srf, NULL, omc->display, &offset);

  if (SDL_MUSTLOCK (omc->display))
    SDL_UnlockSurface (omc->display);

  return 0;
}

static int
display_handler (void *data)
{
  next_time = SDL_GetTicks() + TICK_INTERVAL;
  
  while (1)
  {
    /* update screen composition (i.e. blit surfaces) */
    if (omc->scr)
    {
      widget_t **widgets;
      int depth;

      for (depth = 0; depth < MAX_DEPTH; depth++)
        for (widgets = omc->scr->wlist; *widgets; widgets++)
          if ((*widgets)->layer == depth)
            widget_draw (*widgets);
    }    

    /* flip screen for double buffering */
    SDL_Flip (omc->display);

    /* wait for next interval */
    SDL_Delay (time_left ());
    next_time += TICK_INTERVAL;
  }

  return 0;
}

void
create_display_thread (void)
{
  if (!omc->dth)
    omc->dth = SDL_CreateThread (display_handler, NULL);
}
