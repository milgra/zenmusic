#ifndef ui_manager_h
#define ui_manager_h

#include "view.c"
#include "wm_event.c"

void ui_manager_init(int width, int height);
void ui_manager_event(ev_t event);
void ui_manager_add(view_t* view);
void ui_manager_rem(view_t* view);
void ui_manager_render();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"
#include "mtvector.c"
#include "ui_connector.c"

mtmap_t* viewm; // view map
mtvec_t* viewv; // view vector

void ui_manager_init(int width, int height)
{
  ui_connector_init(width, height);

  viewm = mtmap_alloc();
  viewv = mtvec_alloc();
}

void ui_manager_event(ev_t ev)
{
  view_t* view;
  if ((view = VNXT(viewv)))
  {
    view_evt(view, ev);
  }
}

void ui_manager_add(view_t* view)
{
  ui_connector_add(view);
  VADD(viewv, view);
}

void ui_manager_rem(view_t* view)
{
  ui_connector_rem(view);
  VREM(viewv, view);
}

void ui_manager_render()
{
  ui_connector_render();
}

#endif
