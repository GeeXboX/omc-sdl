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
#include <string.h>

#include "omc.h"
#include "screen.h"
#include "widgets/widget.h"

static int
get_list_length (void *list)
{
  void **l = list;
  int n = 0;

  while (*(l++))
    n++;

  return n;
}

void
screen_uninit (screen_t *screen)
{
  widget_t **widgets;
  
  if (!screen)
    return;

  if (screen->uninit)
    screen->uninit (screen);

  /* free widgets */
  for (widgets = screen->wlist; *widgets; widgets++)
    widget_free (*widgets);
  free (screen->wlist);
  
  free (screen);
}

void
screen_init (screen_type_t type)
{
  screen_t *screen;
  extern void * screen_main_init (screen_t *screen);

  screen = malloc (sizeof (screen_t));
  screen->wlist = (widget_t **) malloc (sizeof (widget_t *));
  *(screen->wlist) = NULL;
  screen->current = NULL;
  screen->priv = NULL;
  screen->handle_event = NULL;
  screen->uninit = NULL;
  
  switch (type)
  {
  case SCREEN_TYPE_MAIN:
    screen_main_init (screen);
    break;
  }

  /* new current screen */
  omc->scr = screen;
}

void
screen_switch (screen_type_t type)
{
  if (omc->scr)
    screen_uninit (omc->scr);

  screen_init (type);
}

void
screen_add_widget (screen_t *screen, widget_t *widget)
{
  widget_t **widgets;
  int n;
  
  if (!screen || !widget)
    return;

  for (widgets = screen->wlist; *widgets; widgets++)
    if (*widgets == widget)
      return; /* already in list */

  /* add widget to list */
  n = get_list_length ((void *) screen->wlist) + 1;
  screen->wlist = (widget_t **)
    realloc (screen->wlist, (n + 1) * sizeof (*(screen->wlist)));
  screen->wlist[n] = NULL;
  screen->wlist[n - 1] = widget;
}
