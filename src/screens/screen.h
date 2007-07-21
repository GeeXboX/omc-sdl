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

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <SDL.h>
#include "widgets/widget.h"

typedef enum {
  SCREEN_TYPE_MAIN
} screen_type_t;

typedef struct screen_s {
  screen_type_t type;
  widget_t **wlist;
  void *priv;
  int (*handle_event) (struct screen_s *screen, SDL_Event *ev);
  void (*uninit) (struct screen_s *screen);
} screen_t;

void screen_init (screen_type_t type);
void screen_uninit (screen_t *screen);
void screen_switch (screen_type_t type);

void screen_add_widget (screen_t *screen, widget_t *widget);

#endif /* _SCREEN_H_ */
