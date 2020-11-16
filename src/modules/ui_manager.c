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
#include "mtmath2.c"
#include "mtvector.c"
#include "ui_generator.c"
#include <limits.h>
#include <math.h>

view_t* root;

void ui_manager_init(int width, int height)
{
  ui_generator_init(width, height);

  root = view_new("root", (vframe_t){0, 0, width, height});
}

void ui_manager_layout(view_t* view)
{
  printf("LAYOUT MAIN %s\n", view->id);
  view_t* v;

  float act_x = 0;
  float act_y = 0;
  float rel_w = view->frame.local.w; // remaining width for relative views
  float rel_h = view->frame.local.h; // remaining height for relative views
  int   rem_w = view->views->length; // remaining relative views for width
  int   rem_h = view->views->length; // remaining relative views for height

  if (view->layout.display == LD_FLEX)
  {
    while ((v = VNXT(view->views)))
    {
      if (v->layout.width > 0)
      {
        rel_w -= v->layout.width;
        rem_w -= 1;
      }
    }
    while ((v = VNXT(view->views)))
    {
      if (v->layout.height > 0)
      {
        rel_h -= v->layout.height;
        rem_h -= 1;
      }
    }
  }

  printf("rel_w %f rem_w %i\n", rel_w, rem_w);

  while ((v = VNXT(view->views)))
  {
    printf("LAYOUT SUB %s\n", v->id);

    vframe_t frame = v->frame.local;

    if (v->layout.width > 0)
    {
      frame.w = v->layout.width;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_ROW)
      {
        frame.x = act_x;
        act_x += frame.w;
        printf("WIDTH rel_w %f act_x %f rem_w %i\n", rel_w, act_x, rem_w);
      }
    }
    if (v->layout.height > 0)
    {
      frame.h = v->layout.height;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_COL)
      {
        frame.y = act_y;
        act_y += frame.h;
        printf("HEIGHT rel_h %f act_y %f rem_h %i\n", rel_h, act_y, rem_h);
      }
    }
    if (v->layout.w_per > 0.0)
    {
      float width = rel_w;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_ROW)
      {
        width   = rel_w / rem_w;
        frame.x = act_x;
        act_x += width;
        rem_w -= 1;
        rel_w -= width;
        printf("W_PER width %f act_x %f rem_w %i\n", width, act_x, rem_w);
      }
      frame.w = width * v->layout.w_per;
    }
    if (v->layout.h_per > 0.0)
    {
      float height = rel_h;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_COL)
      {
        height  = rel_h / rem_h;
        frame.y = act_y;
        act_y += height;
        rem_h -= 1;
        rel_h -= height;
        printf("H_PER height %f act_y %f rem_h %i\n", rel_h, act_y, rem_h);
      }
      frame.h = height * v->layout.h_per;
    }
    if (v->layout.margin == INT_MAX)
    {
      frame.x = (view->frame.local.w / 2.0) - (v->frame.local.w / 2.0);
    }
    if (v->layout.margin_top > 0)
    {
      frame.y += v->layout.margin_top;
      frame.h -= v->layout.margin_top;
    }
    if (v->layout.margin_left > 0)
    {
      frame.x += v->layout.margin_left;
      frame.w -= v->layout.margin_left;
    }
    if (v->layout.margin_right > 0)
    {
      frame.w -= v->layout.margin_right;
    }
    if (v->layout.margin_bottom > 0)
    {
      frame.h -= v->layout.margin_bottom;
    }
    if (v->layout.bottom > 0)
    {
      frame.y = view->frame.local.h - frame.h;
    }
    printf("FINAL %f\n", frame.w);
    view_set_frame(v, frame);
  }

  while ((v = VNXT(view->views)))
  {
    ui_manager_layout(v);
  }
}

void ui_manager_event(ev_t ev)
{
  view_evt(root, ev);

  if (ev.type == EV_RESIZE)
  {
    view_set_frame(root, (vframe_t){0.0, 0.0, (float)ev.w, (float)ev.h});
    ui_generator_resize(ev.w, ev.h);
    ui_manager_layout(root);

    printf("\nAFTER RESIZE");
    view_desc(root, 0);
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
  if (!view->hidden)
  {
    if (view->index != *index)
    {
      view->index = *index;
      if (view->connected == 0)
      {
        //if (!(view->tg == NULL && view->tex == NULL)) //don't add empty views
        ui_generator_add(view);
      }

      ui_generator_set_index(view);
    }
    *index += 1;
  }
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
