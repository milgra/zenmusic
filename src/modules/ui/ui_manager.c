/*
  UI Manager Module for Zen Multimedia Desktop System
  Collects root windows, dispatches events to them, readds them to UI Connector is view chain is changed.
  
 */

#ifndef ui_manager_h
#define ui_manager_h

#include "view.c"
#include "wm_event.c"

void ui_manager_init(int width, int height);
void ui_manager_event(ev_t event);
void ui_manager_add(view_t* view);
void ui_manager_remove(view_t* view);
void ui_manager_render();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"
#include "mtvector.c"
#include "ui_connector.c"

mtmap_t* viewm; /* view map */
mtvec_t* viewv; /* view vector */
char     reindex = 0;

void ui_manager_init(int width, int height)
{
  ui_connector_init(width, height);

  viewm = mtmap_alloc();
  viewv = mtvec_alloc();
}

void ui_manager_event(ev_t ev)
{
  view_t* view;
  while ((view = VNXT(viewv)))
  {
    view_evt(view, ev);
  }

  if (ev.type == EV_RESIZE)
  {
    ui_connector_resize(ev.w, ev.h);
  }
}

void ui_manager_add(view_t* view)
{
  VADD(viewv, view);
  ui_connector_add(view);
  reindex = 1;
}

void ui_manager_remove(view_t* view)
{
  VREM(viewv, view);
  ui_connector_remove(view);
  reindex = 1;
}

void ui_manager_reindex(view_t* view, uint32_t* index)
{
  if (view->index != *index)
  {
    view->index = *index;
    ui_connector_set_index(view);
  }

  *index += 1;

  view_t* v;
  while ((v = VNXT(view->views)))
  {
    ui_manager_reindex(v, index);
  }
}

void ui_manager_render()
{
  if (reindex)
  {
    uint32_t index = 0;
    view_t*  v;
    while ((v = VNXT(viewv)))
    {
      ui_manager_reindex(v, &index);
    }
    reindex = 0;
  }
  ui_connector_render();
}

#endif
