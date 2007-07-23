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
  widget->flags_lock = SDL_CreateMutex ();
  
  widget->x = x;
  widget->y = y;
  widget->w = w;
  widget->h = h;
  widget->layer = layer;
  widget->redraw_area.x = 0;
  widget->redraw_area.y = 0;
  widget->redraw_area.h = 0;
  widget->redraw_area.w = 0;
  
  widget->focus = NULL;
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

int
widget_action (widget_t *widget, action_event_type_t ev)
{
  if (widget && widget->action)
    if (widget_get_flag (widget, WIDGET_FLAG_FOCUSED)) /* widget has focus */
      return widget->action (widget, ev);
  
  return -1;
}

void
widget_free (widget_t *widget)
{
  if (!widget)
    return;

  if (widget->id)
    free (widget->id);

  if (widget->flags_lock)
    SDL_DestroyMutex (widget->flags_lock);

  if (widget->free)
    widget->free (widget);

  free (widget);
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
widget_set_redraw_area (widget_t *widget, int x, int y, int w, int h)
{
  if (!widget)
    return;

  widget->redraw_area.x = x;
  widget->redraw_area.y = y;
  widget->redraw_area.w = w;
  widget->redraw_area.h = h;

  widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 1);
}

int
widget_set_flag (widget_t *widget, widget_flags_t f, int state)
{
  if (!widget)
    return 0;

  SDL_mutexP (widget->flags_lock);
  if (state)
    widget->flags |= f;
  else
    widget->flags &= ~f;
  SDL_mutexV (widget->flags_lock);

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
        if ((widget->x <= ((*widgets)->x + (*widgets)->w))
            || (widget->y <= ((*widgets)->y + (*widgets)->h)))
          /* and tells which specific area needs to be redrawn */
          widget_set_redraw_area (*widgets,
                                  widget->x, widget->y, widget->w, widget->h);
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

neighbours_t *
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

void
neighbours_set (neighbours_t *nb, widget_t *widget, neighbours_type_t type)
{
  if (!nb)
    return;

  switch (type)
  {
  case NEIGHBOURS_UP:
    nb->up = widget;
    break;
  case NEIGHBOURS_DOWN:
    nb->down = widget;
    break;
  case NEIGHBOURS_LEFT:
    nb->left = widget;
    break;
  case NEIGHBOURS_RIGHT:
    nb->right = widget;
    break;
  }
}

void
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
