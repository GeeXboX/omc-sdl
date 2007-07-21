/* GeeXboX Open Media Center.
 * Copyright (C) 2007 Guillaume Lecerf <fox@geexbox.org>
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
#include <SDL_ttf.h>

#include "omc.h"
#include "widget.h"
#include "display.h"

typedef struct widget_text_s {
  SDL_Surface *txt;     /* regular text */
  SDL_Color color;
  SDL_Color fcolor;
  TTF_Font *font;
  char *str;
} widget_text_t;

static TTF_Font *
font_load (char *filename, int size, int style)
{
  TTF_Font *font;

  if (!TTF_WasInit ())
    TTF_Init ();

  font = TTF_OpenFont (filename, size);
  if (!font)
    return NULL;

  TTF_SetFontStyle (font, style);

  return font;
}

static SDL_Surface *
text_create (TTF_Font *font, char *str, SDL_Color color)
{
  SDL_Surface *txt;

  txt = TTF_RenderUTF8_Blended (font, str, color);
  if (!txt)
  {
    printf (SDL_GetError());
    return NULL;
  }

  return txt;
}

static int
widget_text_draw (widget_t *widget)
{
  widget_text_t *priv = (widget_text_t *) widget->priv;
  SDL_Rect dst;

  dst.x = widget->x;
  dst.y = widget->y;
  dst.w = priv->txt->w;
  dst.h = priv->txt->h;
  
  return surface_blit (priv->txt, dst);
}

static int
widget_text_action (widget_t *widget, action_event_type_t ev)
{
  return -1;
}

static void
widget_text_free (widget_t *widget)
{
  widget_text_t *priv;

  if (!widget)
    return;

  priv = (widget_text_t *) widget->priv;

  if (priv->txt)
    SDL_FreeSurface (priv->txt);

  if (priv->font)
    TTF_CloseFont (priv->font);

  if (priv->str)
    free (priv->str);
  
  free (priv);
}

widget_t *
text_new (char *id, int focusable, int show, int layer,
           char *name, char *fontname, int size,
           int r, int g, int b, int rf, int gf, int bf,
           int x, int y, int w, int h)
{
  widget_t *widget = NULL;
  widget_text_t *priv = NULL;
  int flags = WIDGET_FLAG_NONE;

  if (!fontname || !name)
    return NULL;
  
  if (show)
    flags |= WIDGET_FLAG_SHOW;

  if (focusable)
    flags |= WIDGET_FLAG_FOCUSABLE;

  widget = widget_new (id, WIDGET_TYPE_TEXT, flags, layer, x, y, w, h);

  priv = malloc (sizeof (widget_text_t));
  printf ("Loading \"%s\"\n", name);
  priv->font = font_load (fontname, size, TTF_STYLE_NORMAL);

  priv->color.r = r;
  priv->color.g = g;
  priv->color.b = b;
  priv->color.unused = 255;

  priv->fcolor.r = rf;
  priv->fcolor.g = gf;
  priv->fcolor.b = bf;
  priv->fcolor.unused = 255;
  
  priv->str = strdup (name);
  priv->txt = text_create (priv->font, priv->str, priv->color);

  widget->priv = priv;

  widget->draw = widget_text_draw;
  widget->set_focus = NULL;
  widget->action = widget_text_action;
  widget->free = widget_text_free;

  return widget;
}
