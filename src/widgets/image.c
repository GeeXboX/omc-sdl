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
#include "display.h"

typedef struct widget_image_s {
  SDL_Surface *img;
  char *name;           /* regular image */
  char *fname;          /* focused image */
} widget_image_t;

static SDL_Surface *
image_load (char *filename, int w, int h)
{
  SDL_Surface *img, *img2;

  if (!filename)
    return NULL;
  
  img = IMG_Load (filename);
  if (!img)
  {
    printf("ERROR: unable to load image file [%s]\n", filename);
    return NULL;
  }
  printf ("Loaded a %d x %d image\n", img->w, img->h);

  /* converts surface to display format once for all */
  img2 = SDL_DisplayFormatAlpha (img);
  if (img2)
  {
    SDL_FreeSurface (img);
    img = img2;
  }
  
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
widget_image_draw (widget_t *widget)
{
  widget_image_t *priv = (widget_image_t *) widget->priv;
  SDL_Rect dst;

  if (!priv->img)
    return -1;

  dst.x = widget->x;
  dst.y = widget->y;
  dst.w = priv->img->w;
  dst.h = priv->img->h;
  
  return surface_blit (widget, priv->img, dst);
}

static int
widget_image_set_focus (struct widget_s *widget)
{
  widget_image_t *priv = (widget_image_t *) widget->priv;

  if (priv->img)
    SDL_FreeSurface (priv->img);

  if (widget_get_flag (widget, WIDGET_FLAG_FOCUSED))
    priv->img = image_load (priv->name, widget->w, widget->h);
  else
    priv->img = image_load (priv->fname, widget->w, widget->h);

  if(!priv->img)
    return 1;

  widget->w = priv->img->w;
  widget->h = priv->img->h;
  
  return 0;
}

static int
widget_image_action (widget_t *widget, action_event_type_t ev)
{
  return widget_action_default_cb (widget, ev);
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

  if (priv->name)
    free (priv->name);
  if (priv->fname)
    free (priv->fname);
  
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

  if (show)
    flags |= WIDGET_FLAG_SHOW;
    
  if (focusable)
    flags |= WIDGET_FLAG_FOCUSABLE;
  
  widget = widget_new (id, WIDGET_TYPE_IMAGE, flags, layer, x, y, w, h);

  priv = malloc (sizeof (widget_image_t));
  printf ("Loading %s\n", name);
  priv->name = name ? strdup (name) : NULL;
  priv->fname = fname ? strdup (fname) : NULL;
  priv->img = image_load (priv->name, w, h);

  if(!priv->img)
    return NULL;

  widget->w = priv->img->w;
  widget->h = priv->img->h;
  
  widget->priv = priv;

  widget->draw = widget_image_draw;
  widget->set_focus = widget_image_set_focus;
  widget->action = widget_image_action;
  widget->free = widget_image_free;

  return widget;
}

void
image_set_picture (widget_t *widget, char *name)
{
  widget_image_t *priv;

  if (!widget || !name)
    return;
  
  priv = (widget_image_t *) widget->priv;
  
  if (priv->img)
    SDL_FreeSurface (priv->img);

  priv->img = image_load (name, widget->w, widget->h);

  if(!priv->img)
    return;

  widget->w = priv->img->w;
  widget->h = priv->img->h;
  
  widget_set_flag (widget, WIDGET_FLAG_NEED_REDRAW, 1);
}
