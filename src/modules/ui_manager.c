/*
  UI Manager Module for Zen Multimedia Desktop System
  Collects root windows, dispatches events to them, readds them to UI Generator is view chain is changed.
  
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
#include "ui_generator.c"

view_t* root;

void ui_manager_init(int width, int height)
{
  ui_generator_init(width, height);

  root = view_new("root", (vframe_t){0, 0, width, height}, 0);
}

void ui_manager_event(ev_t ev)
{

  view_evt(root, ev);

  if (ev.type == EV_RESIZE)
  {
    view_set_frame(root, (vframe_t){0, 0, ev.w, ev.h});
    ui_generator_resize(ev.w, ev.h);
  }
}

void ui_manager_add(view_t* view)
{
  view_add(root, view);
}

void ui_manager_remove(view_t* view)
{
  view_remove(root, view);
}

void ui_manager_reindex(view_t* view, uint32_t* index)
{
  if (view->index != *index)
  {
    view->index = *index;
    if (view->connected == 0)
    {
      ui_generator_add(view);
    }

    ui_generator_set_index(view);
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
    ui_generator_cleanup();
    uint32_t index = 0;
    ui_manager_reindex(root, &index);
    reindex = 0;
  }
  ui_generator_render();
}

#endif
