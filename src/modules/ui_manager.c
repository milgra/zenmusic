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
void ui_manager_render(uint32_t time);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"
#include "mtmath2.c"
#include "mtvector.c"
#include "ui_generator.c"
#include "view_layout.c"

view_t* root;
vec_t*  queue;

void ui_manager_init(int width, int height)
{
  ui_generator_init(width, height);

  root  = view_new("root", (r2_t){0, 0, width, height});
  queue = VNEW();
}

void ui_manager_event(ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    view_evt(root, ev);
  }
  else if (ev.type == EV_RESIZE)
  {
    r2_t rf = root->frame.local;
    if (rf.w != (float)ev.w ||
        rf.h != (float)ev.h)
    {
      view_set_frame(root, (r2_t){0.0, 0.0, (float)ev.w, (float)ev.h});
      view_layout(root);
      ui_generator_resize(ev.w, ev.h);
      /* printf("\nAFTER RESIZE"); */
      /* view_desc(root, 0); */
      view_evt(root, ev);
    }
  }
  else if (ev.type == EV_MMOVE ||
           ev.type == EV_MDOWN ||
           ev.type == EV_MUP)
  {
    if (ev.type == EV_MMOVE || queue->length == 0)
    {
      // send mouse move event to previous queue to detect move out
      for (int i = queue->length - 1; i > -1; i--)
      {
        view_t* v = queue->data[i];
        if (v->needs_touch)
        {
          if (v->handler) (*v->handler)(v, ev);
          break;
        }
      }

      vec_reset(queue);
      view_coll_touched(root, ev, queue);
    }

    for (int i = queue->length - 1; i > -1; i--)
    {
      view_t* v = queue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    for (int i = queue->length - 1; i > -1; i--)
    {
      view_t* v = queue->data[i];
      if (v->needs_scroll)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
  else if (ev.type == EV_KDOWN || ev.type == EV_KUP)
  {
    for (int i = queue->length - 1; i > -1; i--)
    {
      view_t* v = queue->data[i];
      if (v->needs_key)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
  else if (ev.type == EV_TEXT)
  {
    for (int i = queue->length - 1; i > -1; i--)
    {
      view_t* v = queue->data[i];
      if (v->needs_text)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
}

void ui_manager_add(view_t* view)
{
  view_add(root, view);
}

void ui_manager_resend(view_t* view)
{
  if (view->display) ui_generator_add(view);
  vec_t* vec = view->views;
  for (int i = 0; i < vec->length; i++) ui_manager_resend(vec->data[i]);
}

void ui_manager_render(uint32_t time)
{
  if (resend)
  {
    ui_generator_cleanup();
    ui_manager_resend(root);
    resend = 0;
  }
  ui_generator_render(time);
}

#endif
