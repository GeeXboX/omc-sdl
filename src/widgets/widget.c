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
  widget->x = x;
  widget->y = y;
  widget->w = w;
  widget->h = h;
  widget->layer = layer;

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
  if (!(widget->flags & WIDGET_FLAG_NEED_REDRAW))
    return -1;
   
  widget->draw (widget);
  widget->flags =~ WIDGET_FLAG_NEED_REDRAW; /* widget has been drawn */

  return 0;
}

int
widget_show (widget_t *widget)
{
  if (!widget)
    return -1;

  /* show only makes sense when currently hidden */
  if (!(widget->flags & WIDGET_FLAG_SHOW))
    return -1;

  widget->flags |= WIDGET_FLAG_SHOW; /* show */
  widget->flags |= WIDGET_FLAG_NEED_REDRAW; /* trigger redraw */

  return 0;
}

int
widget_hide (widget_t *widget)
{
  if (!widget)
    return -1;

  /* hide only makes sense when currently shown */
  if (widget->flags & WIDGET_FLAG_SHOW)
    return -1;

  widget->flags =~ WIDGET_FLAG_SHOW; /* hide */
  widget->flags |= WIDGET_FLAG_NEED_REDRAW; /* trigger redraw */

  return 0;
}

int
widget_set_focus (widget_t *widget, int state)
{
  if (!widget || !(widget->flags & WIDGET_FLAG_FOCUSABLE))
    return -1;

  if (state)
    widget->flags |= WIDGET_FLAG_FOCUSED;
  else
    widget->flags &= WIDGET_FLAG_FOCUSED;
  
  if (widget->set_focus)
    return widget->set_focus (widget);

  return -1;
}

int
widget_action (widget_t *widget, action_event_type_t ev)
{
  if (widget && widget->action)
    if (widget->flags & WIDGET_FLAG_FOCUSED) /* widget has focus */
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

  if (widget->free)
    widget->free (widget);

  free (widget);
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
