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

#include <stdlib.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include "omc.h"
#include "widget.h"

typedef struct widget_image_s {
  SDL_Surface *img;     /* regular image */
  SDL_Surface *fimg;    /* focused image */
} widget_image_t;

static SDL_Surface *
image_load (char *filename, int w, int h)
{
  SDL_Surface *img, *img2;

  if (!filename)
    return NULL;
  
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

  SDL_FreeSurface (img);
  return img2;
}

static int
surface_blit (widget_t *widget, int x, int y)
{
  widget_image_t *priv = (widget_image_t *) widget->priv;
  SDL_Surface *srf;
  SDL_Rect src, dest;
  
  if (!widget)
    return -1;

  srf = widget->flags & WIDGET_FLAG_FOCUSED ? priv->fimg : priv->img;

  if (!srf)
    return -1;
  
  src.x = 0;
  src.y = 0;
  src.w = srf->w;
  src.h = srf->h;

  dest = src;
  dest.x = widget->x;
  dest.y = widget->y;

  SDL_BlitSurface (srf, &src, omc->display, &dest);
  SDL_UpdateRect (omc->display, dest.x, dest.y, dest.w, dest.h);

  return 0;
}

static int
widget_image_show (widget_t *widget)
{
  return surface_blit (widget, widget->x, widget->y);
}

static int
widget_image_hide (widget_t *widget)
{
  return surface_blit (widget, 0, 0);
}

static int
widget_image_action (widget_t *widget, action_event_type_t ev)
{
  return -1;
}

static void
widget_image_free (widget_t *widget)
{
  widget_image_t *priv;
  
  if (!widget)
    return;

  priv = (widget_image_t *) widget->priv;

  if (priv->img)
    SDL_FreeSurface (priv->img);
  if (priv->fimg)
    SDL_FreeSurface (priv->fimg);
  free (priv);
}

widget_t *
image_new (char *id, int focusable, int show, int layer,
           char *name, char *fname,
           int x, int y, int w, int h)
{
  widget_t *widget = NULL;
  widget_image_t *priv = NULL;
  int flags = WIDGET_FLAG_NONE;

  if (focusable)
    flags |= WIDGET_FLAG_FOCUSABLE;
  
  widget = widget_new (id, WIDGET_TYPE_IMAGE, flags, layer, x, y, w, h);

  priv = malloc (sizeof (widget_image_t));
  printf ("Loading %s\n", name);
  priv->img = image_load (name, w, h);
  priv->fimg = image_load (fname, w, h);

  widget->priv = priv;

  widget->show = widget_image_show;
  widget->hide = widget_image_hide;
  widget->set_focus = NULL;
  widget->action = widget_image_action;
  widget->free = widget_image_free;

  return widget;
}
