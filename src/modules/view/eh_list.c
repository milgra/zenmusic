#ifndef eh_list_h
#define eh_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_list_t
{
  mtvec_t* items;
  mtvec_t* cache;

  int  head_index;
  int  tail_index;
  char filled;

  int count;

  view_t* vscr;
  view_t* hscr;

  view_t* (*row_generator)(view_t* listview, view_t* rowview, int index); /* event handler for view */
} eh_list_t;

void eh_list_add(view_t* view, view_t* (*row_generator)(view_t* listview, view_t* rowview, int index));
void eh_list_fill(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_css.c"
#include "tg_text.c"
#include <math.h>

#define PRELOAD_DISTANCE 100.0

void eh_list_move(view_t* view, float dy)
{
  eh_list_t* eh = view->evt_han_data;
  view_t*    sview;
  while ((sview = VNXT(eh->items)))
  {
    r2_t frame = sview->frame.local;
    frame.y += dy;
    view_set_frame(sview, frame);
  }
  view_set_frame(eh->vscr, (r2_t){view->frame.local.w - 20.0, 0.0, 20.0, 20.0});
  view_set_frame(eh->hscr, (r2_t){0, view->frame.local.h - 20.0, 20.0, 20.0});
}

void eh_list_evt(view_t* view, ev_t ev)
{
  eh_list_t* eh = view->evt_han_data;
  if (ev.type == EV_TIME)
  {
    // fill up if needed
    while (eh->filled == 0)
    {
      if (eh->items->length == 0)
      {
        view_t* cacheitem = mtvec_head(eh->cache);
        view_t* rowitem   = (*eh->row_generator)(view, cacheitem, 0);

        if (rowitem)
        {
          VREM(eh->cache, rowitem);
          VADD(eh->items, rowitem);
          view_insert(view, rowitem, 0);
        }
        else
          eh->filled = 1;
      }
      else
      {
        view_t* head = mtvec_head(eh->items);
        view_t* tail = mtvec_tail(eh->items);

        // add items if needed

        if (head->frame.local.y > 0.0 - PRELOAD_DISTANCE)
        {
          view_t* cacheitem = mtvec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->head_index - 1);

          if (rowitem)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            mtvec_addatindex(eh->items, rowitem, 0);

            if (rowitem->parent == NULL) view_insert(view, rowitem, 0);
            view_set_frame(rowitem, (r2_t){0, head->frame.local.y - rowitem->frame.local.h, rowitem->frame.local.w, rowitem->frame.local.h});

            eh->head_index -= 1;
          }
          else
            eh->filled = 1;
        }
        else
          eh->filled = 1;

        if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h + PRELOAD_DISTANCE)
        {
          view_t* cacheitem = mtvec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->tail_index + 1);

          if (rowitem)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            VADD(eh->items, rowitem);

            if (rowitem->parent == NULL) view_insert(view, rowitem, view->views->length - 3);
            view_set_frame(rowitem, (r2_t){0, tail->frame.local.y + tail->frame.local.h, rowitem->frame.local.w, rowitem->frame.local.h});

            eh->tail_index += 1;
          }
          else
            eh->filled &= 1;
        }
        else
          eh->filled &= 1;

        // remove items if needed

        if (head->frame.local.y + head->frame.local.h < 0.0 - PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, head);

          VREM(eh->items, head);
          //view_remove(view, head);

          eh->head_index += 1;
        }

        if (tail->frame.local.y > view->frame.local.h + PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, tail);

          VREM(eh->items, tail);
          //view_remove(view, tail);

          eh->tail_index -= 1;
        }
      }
    }
    // scroll bounce if needed
    if (eh->items->length > 0)
    {
      view_t* head = mtvec_head(eh->items);
      view_t* tail = mtvec_tail(eh->items);

      // add items if needed

      if (head->frame.local.y > 0.0001)
        eh_list_move(view, -head->frame.local.y / 5.0);
      else if (head->frame.local.y < -0.0001 && tail->frame.local.y + tail->frame.local.h < view->frame.local.h)
        eh_list_move(view, (view->frame.local.h - tail->frame.local.h - tail->frame.local.y) / 5.0);
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    eh_list_move(view, ev.dy);
    eh->filled = 0;
  }
  else if (ev.type == EV_RESIZE)
  {
    eh->filled = 0;
  }
}

void eh_list_del(void* p)
{
  eh_list_t* eh = (eh_list_t*)p;
  REL(eh->items);
}

void eh_list_fill(view_t* view)
{
  eh_list_t* eh = view->evt_han_data;
  eh->filled    = 0;
}

void eh_list_add(view_t* view, view_t* (*row_generator)(view_t* listview, view_t* rowview, int index))
{
  eh_list_t* eh     = mtmem_calloc(sizeof(eh_list_t), "eh_list", eh_list_del, NULL);
  eh->items         = VNEW();
  eh->cache         = VNEW();
  eh->row_generator = row_generator;

  view_t* vscr = view_new("vscr", (r2_t){0, 0, 20, 20});
  view_t* hscr = view_new("hscr", (r2_t){0, 20, 20, 20});

  tg_css_add(vscr);
  tg_css_add(hscr);

  vscr->layout.background_color = 0x000000FF;
  hscr->layout.background_color = 0x000000FF;

  view_add(view, vscr);
  view_add(view, hscr);

  eh->vscr = vscr;
  eh->hscr = hscr;

  view->needs_scroll = 1;
  view->evt_han_data = eh;
  view->evt_han      = eh_list_evt;
}

#endif
