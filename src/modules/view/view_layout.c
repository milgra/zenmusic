#ifndef view_layout_h
#define view_layout_h

#include "view.c"

void view_layout(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <limits.h>
#include <math.h>

void view_layout(view_t* view)
{
  //printf("LAYOUT MAIN %s\n", view->id);
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

  while ((v = VNXT(view->views)))
  {
    r2_t frame = v->frame.local;

    if (v->layout.margin > 0)
    {
      frame.x = 0;
      frame.y = 0;
    }

    if (v->layout.width > 0)
    {
      frame.w = v->layout.width;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_ROW)
      {
        frame.x = act_x;
        act_x += frame.w;
      }
    }
    if (v->layout.height > 0)
    {
      frame.h = v->layout.height;
      if (view->layout.display == LD_FLEX && view->layout.flexdir == FD_COL)
      {
        frame.y = act_y;
        act_y += frame.h;
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
      }
      frame.h = height * v->layout.h_per;
    }
    if (v->layout.margin == INT_MAX || view->layout.cjustify == JC_CENTER)
    {
      frame.x = (view->frame.local.w / 2.0) - (v->frame.local.w / 2.0);
    }
    if (view->layout.itemalign == IA_CENTER)
    {
      frame.y = (view->frame.local.h / 2.0) - (v->frame.local.h / 2.0);
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
    if (v->layout.top > 0)
    {
      frame.y = v->layout.top - 1;
    }
    if (v->layout.left > 0)
    {
      frame.x = v->layout.left - 1;
    }
    if (v->layout.right > 0)
    {
      frame.x = view->frame.local.w - frame.w - v->layout.right + 1;
    }
    if (v->layout.bottom > 0)
    {
      frame.y = view->frame.local.h - frame.h - v->layout.bottom + 1;
    }
    view_set_frame(v, frame);
  }

  while ((v = VNXT(view->views)))
  {
    view_layout(v);
  }
}

#endif
