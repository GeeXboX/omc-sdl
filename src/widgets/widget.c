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
#include <stdlib.h>
#include <string.h>

#include "omc.h"
#include "widget.h"

widget_t *
widget_new (char *id, widget_type_t type, int flags, uint8_t layer,
            uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  widget_t *widget = NULL;

  if (!id) /* mandatory */
    return NULL;
  
  widget = malloc (sizeof (widget_t));
  widget->id = strdup (id);
  widget->type = type;
  widget->flags = flags;
  widget->lock = SDL_CreateMutex ();
  
  widget->x = x;
  widget->y = y;
  widget->w = w;
  widget->h = h;
  widget->layer = layer;
  widget->redraw_area.x = 0;
  widget->redraw_area.y = 0;
  widget->redraw_area.h = 0;
  widget->redraw_area.w = 0;
  
  widget->nb = NULL;
  widget->priv = NULL;
  widget->draw = NULL;
  widget->set_focus = NULL;
  widget->action = NULL;
  widget->free = NULL;

  if (flags & WIDGET_FLAG_SHOW)
    widget->flags |= WIDGET_FLAG_NEED_REDRAW;
  
  return widget;
}

int
widget_draw (widget_t *widget)
{
  if (!widget || !widget->draw)
    return -1;

  /* check if widget really needs to be redrawn */
  if (!widget_get_flag (widget, WIDGET_FLAG_NEED_REDRAW))
    return -1;
   
  widget->draw (widget);

  /* widget has been drawn */
  widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 0);
  
  return 0;
}

int
widget_show (widget_t *widget)
{
  if (!widget)
    return -1;

  /* show only makes sense when currently hidden */
  if (!widget_get_flag (widget, WIDGET_FLAG_SHOW))
    return -1;

  /* show & trigger redraw */
  widget_set_flag (widget, WIDGET_FLAG_SHOW | WIDGET_FLAG_NEED_REDRAW, 1);
  
  return 0;
}

int
widget_hide (widget_t *widget)
{
  if (!widget)
    return -1;

  /* hide only makes sense when currently shown */
  if (widget_get_flag (widget, WIDGET_FLAG_SHOW))
    return -1;

  widget_set_flag (widget, WIDGET_FLAG_SHOW, 0); /* hide */
  widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 1); /* trigger redraw */
  
  return 0;
}

int
widget_set_focus (widget_t *widget, int state)
{
  if (!widget || !widget_get_flag (widget, WIDGET_FLAG_FOCUSABLE))
    return -1;

  widget_set_flag (widget, WIDGET_FLAG_FOCUSED, state);

  printf ("Widget '%s' now %s focus.\n", widget->id,
          widget_get_flag (widget, WIDGET_FLAG_FOCUSED) ? "has" : "lost");
  
  if (widget->set_focus)
  {
    int r = widget->set_focus (widget);
    widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 1);
    return r;
  }

  return -1;
}

static widget_t * widget_get_neighbour (widget_t *widget,
                                        neighbours_type_t type);

int
widget_move_focus (widget_t *widget, neighbours_type_t where)
{
  widget_t *w;

  if (!widget || !widget_get_flag (widget, WIDGET_FLAG_FOCUSED))
    return -1;

  w = widget_get_neighbour (widget, where);
  if (!w)
    return -1;

  /* current widget loose focus */
  widget_set_focus (widget, 0);

  /* his neighbours gains focus */
  widget_set_focus (w, 1);
  omc->scr->current = w;

  return 0;
}

int
widget_action_default_cb (widget_t *widget, action_event_type_t ev)
{
  if (!widget)
    return -1;
  
  switch (ev)
  {
  case ACTION_EVENT_GO_UP:
    widget_move_focus (widget, NEIGHBOURS_UP);
    break;
  case ACTION_EVENT_GO_DOWN:
    widget_move_focus (widget, NEIGHBOURS_DOWN);
    break;
  case ACTION_EVENT_GO_LEFT:
    widget_move_focus (widget, NEIGHBOURS_LEFT);
    break;
  case ACTION_EVENT_GO_RIGHT:
    widget_move_focus (widget, NEIGHBOURS_RIGHT);
    break;
  case ACTION_EVENT_CANCEL:
    printf ("[%s], cancelling action\n", widget->id);
    break;
  case ACTION_EVENT_OK:
    printf ("[%s], performing action\n", widget->id);
    break;
  }
  
  return 0;
}

int
widget_action (widget_t *widget, action_event_type_t ev)
{
  if (widget)
    if (widget_get_flag (widget, WIDGET_FLAG_FOCUSED)) /* widget has focus */
    {
      if (widget->action)
        return widget->action (widget, ev); /* widget specific action cb */
      else
        return widget_action_default_cb (widget, ev); /* generic action cb */
    }
  
  return -1;
}

widget_t *
widget_get_by_id (widget_t **list, char *id)
{
  widget_t **w;

  if (!list)
    return NULL;

  for (w = list; *w; w++)
    if (!strcmp ((*w)->id, id))
      return (*w);

  return NULL;
}

static void
widget_set_redraw_area (widget_t *widget, SDL_Rect area)
{
  if (!widget)
    return;

  SDL_mutexP (widget->lock);
  widget->redraw_area.x = area.x;
  widget->redraw_area.y = area.y;
  widget->redraw_area.w = area.w;
  widget->redraw_area.h = area.h;
  SDL_mutexV (widget->lock);
  
  widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 1);
}

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

static int
widget_share_area (SDL_Rect r1, SDL_Rect r2, SDL_Rect *area)
{
  // check if the rectangles intersect
  if( (r2.x + r2.w < r1.x) || (r2.x > r1.x + r1.w) ||
      (r2.y + r2.h < r1.y) || (r2.y > r1.y + r1.h) )
    return 0;

  area->x = MAX(r1.x, r2.x);
  area->y = MAX(r1.y, r2.y);
  area->w = MIN(r1.x + r1.w, r2.x + r2.w) - area->x;
  area->h = MIN(r1.y + r1.h, r2.y + r2.h) - area->y;

  return 1;
}

int
widget_set_flag (widget_t *widget, widget_flags_t f, int state)
{
  if (!widget)
    return 0;

  SDL_mutexP (widget->lock);
  if (state)
    widget->flags |= f;
  else
    widget->flags &= ~f;
  SDL_mutexV (widget->lock);

  /* special care for 'need redraw' flag */
  if (omc->scr && widget_get_flag (widget, WIDGET_FLAG_NEED_REDRAW))
  {
    widget_t **widgets;

    /* for all widgets from screen ... */
    for (widgets = omc->scr->wlist; *widgets; widgets++)
    {
      /* select those underneath the current widget ... */
      if ((*widgets != widget) && (*widgets)->layer < widget->layer)
      {
        /* which share some display area with the current one ... */
        SDL_Rect area;
        SDL_Rect r1 = { widget->x, widget->y, widget->w, widget->h };
        SDL_Rect r2 = { (*widgets)->x, (*widgets)->y,
                        (*widgets)->w, (*widgets)->h };
        if (widget_share_area (r1, r2, &area)
            || widget_share_area (r2, r1, &area))
        {
          printf ("Common area between %s (%d,%d,%d,%d) and %s (%d,%d,%d,%d)"
                  " : (%d x %d) - (%d x %d)\n",
                  widget->id,
                  widget->x, widget->y, widget->w, widget->h,
                  (*widgets)->id,
                  (*widgets)->x, (*widgets)->y, (*widgets)->w, (*widgets)->h,
                  area.x, area.y, area.x + area.w, area.y + area.h);
          widget_set_redraw_area (*widgets, area);
        } 
      }
    }
    
  }

  return 1;
}

int
widget_get_flag (widget_t *widget, widget_flags_t f)
{
  if (!widget)
    return 0;

  if (widget->flags & f)
    return 1;

  return 0;
}

static neighbours_t *
neighbours_new (void)
{
  neighbours_t *nb = NULL;

  nb = malloc (sizeof (neighbours_t));
  nb->up = NULL;
  nb->down = NULL;
  nb->left = NULL;
  nb->right = NULL;

  return nb;
}

static void
neighbours_free (neighbours_t *nb)
{
  if (!nb)
    return;

  nb->up = NULL;
  nb->down = NULL;
  nb->left = NULL;
  nb->right = NULL;

  free (nb);
}

void
widget_set_neighbour (widget_t *widget, widget_t *w, neighbours_type_t type)
{
  if (!widget || !w)
    return;

  if (!widget->nb)
    widget->nb = neighbours_new ();
    
  switch (type)
  {
  case NEIGHBOURS_UP:
    widget->nb->up = w;
    break;
  case NEIGHBOURS_DOWN:
    widget->nb->down = w;
    break;
  case NEIGHBOURS_LEFT:
    widget->nb->left = w;
    break;
  case NEIGHBOURS_RIGHT:
    widget->nb->right = w;
    break;
  }
}

static widget_t *
widget_get_neighbour (widget_t *widget, neighbours_type_t type)
{
  if (!widget || !widget->nb)
    return NULL;

  switch (type)
  {
  case NEIGHBOURS_UP:
    return widget->nb->up;
  case NEIGHBOURS_DOWN:
    return widget->nb->down;
  case NEIGHBOURS_LEFT:
    return widget->nb->left;
  case NEIGHBOURS_RIGHT:
    return widget->nb->right;
  }

  return NULL;
}

void
widget_free (widget_t *widget)
{
  if (!widget)
    return;

  if (widget->id)
    free (widget->id);

  if (widget->lock)
    SDL_DestroyMutex (widget->lock);

  if (widget->nb)
    neighbours_free (widget->nb);
  
  if (widget->free)
    widget->free (widget);

  free (widget);
}
