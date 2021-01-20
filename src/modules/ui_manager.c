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
void ui_manager_render(uint32_t time);
void ui_manager_activate(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"
#include "mtmath2.c"
#include "mtvector.c"
#include "ui_generator.c"
#include "view_layout.c"

struct _uim_t
{
  view_t* root;
  vec_t*  views;
  vec_t*  touchqueue;
  vec_t*  scrollqueue;
} uim = {0};

void ui_manager_init(int width, int height)
{
  ui_generator_init(width, height);

  uim.root        = view_new("root", (r2_t){0, 0, width, height});
  uim.views       = VNEW();
  uim.touchqueue  = VNEW();
  uim.scrollqueue = VNEW();
}

void ui_manager_event(ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    view_evt(uim.root, ev);
  }
  else if (ev.type == EV_RESIZE)
  {
    r2_t rf = uim.root->frame.local;
    printf("resize %i %i\n", ev.w, ev.h);
    if (rf.w != (float)ev.w ||
        rf.h != (float)ev.h)
    {
      view_set_frame(uim.root, (r2_t){0.0, 0.0, (float)ev.w, (float)ev.h});
      view_layout(uim.root);
      ui_generator_resize(ev.w, ev.h);
      /* printf("\nAFTER RESIZE"); */
      /* view_desc(uim.root, 0); */
      view_evt(uim.root, ev);
    }
  }
  else if (ev.type == EV_MMOVE ||
           ev.type == EV_MDOWN ||
           ev.type == EV_MUP)
  {
    if (ev.type == EV_MDOWN)
    {
      // send mouse move event to previous uim.queue to detect move down outside
      for (int i = uim.touchqueue->length - 1; i > -1; i--)
      {
        ev_t outev = ev;
        outev.type = EV_MDOWN_OUTSIDE;
        view_t* v  = uim.touchqueue->data[i];
        if (v->needs_touch)
        {
          if (v->handler) (*v->handler)(v, outev);
          if (v->blocks_touch) break;
        }
      }

      vec_reset(uim.touchqueue);
      view_coll_touched(uim.root, ev, uim.touchqueue);
    }

    if (uim.scrollqueue->length > 0) vec_reset(uim.scrollqueue);

    for (int i = uim.touchqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.touchqueue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        if (v->blocks_touch) break;
      }
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    if (uim.scrollqueue->length == 0) view_coll_touched(uim.root, ev, uim.scrollqueue);

    for (int i = uim.scrollqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.scrollqueue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        if (v->blocks_touch) break;
      }
    }
  }
  else if (ev.type == EV_KDOWN || ev.type == EV_KUP)
  {
    for (int i = uim.touchqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.touchqueue->data[i];
      if (v->needs_key)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
  else if (ev.type == EV_TEXT)
  {
    for (int i = uim.touchqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.touchqueue->data[i];
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
  view_add(uim.root, view);
}

void ui_manager_remove(view_t* view)
{
  view_remove(uim.root, view);
}

void ui_manager_activate(view_t* view)
{
  VADD(uim.touchqueue, view);
}

void ui_manager_collect(view_t* view, vec_t* views)
{
  if (view->display) VADD(views, view);
  vec_t* vec = view->views;
  for (int i = 0; i < vec->length; i++) ui_manager_collect(vec->data[i], views);
}

void ui_manager_render(uint32_t time)
{
  if (resend)
  {
    vec_reset(uim.views);
    ui_manager_collect(uim.root, uim.views);
    ui_generator_use(uim.views);
    resend = 0;
  }
  ui_generator_render(time);
}

#endif
