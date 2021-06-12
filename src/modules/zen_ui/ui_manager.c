#ifndef ui_manager_h
#define ui_manager_h

#include "view.c"
#include "wm_event.c"

void    ui_manager_init(int width, int height);
void    ui_manager_event(ev_t event);
void    ui_manager_add(view_t* view);
void    ui_manager_remove(view_t* view);
void    ui_manager_render(uint32_t time);
void    ui_manager_activate(view_t* view);
view_t* ui_manager_get_root();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "ui_generator.c"
#include "view_layout.c"
#include "zc_map.c"
#include "zc_math2.c"
#include "zc_vector.c"

struct _uim_t
{
  view_t* root;
  vec_t*  views;
  vec_t*  implqueue; // views selected by roll over
  vec_t*  explqueue; // views selected by click
} uim = {0};

void ui_manager_init(int width, int height)
{
  ui_generator_init(width, height);

  uim.root      = view_new("root", (r2_t){0, 0, width, height});
  uim.views     = VNEW();
  uim.implqueue = VNEW();
  uim.explqueue = VNEW();
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
    if (rf.w != (float)ev.w ||
        rf.h != (float)ev.h)
    {
      view_set_frame(uim.root, (r2_t){0.0, 0.0, (float)ev.w, (float)ev.h});
      view_layout(uim.root);
      ui_generator_resize(ev.w, ev.h);
      // printf("\nAFTER RESIZE");
      // view_desc(uim.root, 0);
      view_evt(uim.root, ev);
    }
  }
  else if (ev.type == EV_MMOVE)
  {
    ev_t outev = ev;
    outev.type = EV_MMOVE_OUT;
    for (int i = uim.implqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.implqueue->data[i];
      if (v->needs_touch)
      {
        if (v->handler) (*v->handler)(v, outev);
        if (v->blocks_touch) break;
      }
    }

    vec_reset(uim.implqueue);
    view_coll_touched(uim.root, ev, uim.implqueue);

    for (int i = uim.implqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.implqueue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        if (v->blocks_touch) break;
      }
    }
  }
  else if (ev.type == EV_MDOWN ||
           ev.type == EV_MUP)
  {
    ev_t outev = ev;
    if (ev.type == EV_MDOWN) outev.type = EV_MDOWN_OUT;
    if (ev.type == EV_MUP) outev.type = EV_MUP_OUT;

    for (int i = uim.explqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.explqueue->data[i];
      if (v->needs_touch)
      {
        if (v->handler) (*v->handler)(v, outev);
        if (v->blocks_touch) break;
      }
    }

    vec_reset(uim.explqueue);
    view_coll_touched(uim.root, ev, uim.explqueue);

    for (int i = uim.explqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.explqueue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        if (v->blocks_touch) break;
      }
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    view_coll_touched(uim.root, ev, uim.implqueue);

    for (int i = uim.implqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.implqueue->data[i];
      if (v->needs_touch && v->parent)
      {
        if (v->handler) (*v->handler)(v, ev);
        if (v->blocks_scroll) break;
      }
    }
  }
  else if (ev.type == EV_KDOWN || ev.type == EV_KUP)
  {
    for (int i = uim.explqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.explqueue->data[i];
      if (v->needs_key)
      {
        if (v->handler) (*v->handler)(v, ev);
        break;
      }
    }
  }
  else if (ev.type == EV_TEXT)
  {
    for (int i = uim.explqueue->length - 1; i > -1; i--)
    {
      view_t* v = uim.explqueue->data[i];
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
  VADD(uim.explqueue, view);
}

void ui_manager_collect(view_t* view, vec_t* views)
{
  if (!view->exclude) VADD(views, view);
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

view_t* ui_manager_get_root()
{
  return uim.root;
}

#endif
