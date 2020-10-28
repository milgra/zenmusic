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

  view_t* (*row_generator)(view_t* listview, view_t* rowview, int index); /* event handler for view */
} eh_list_t;

void eh_list_add(view_t* view, view_t* (*row_generator)(view_t* listview, view_t* rowview, int index));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_text.c"
#include <math.h>

#define PRELOAD_DISTANCE 100.0

void eh_list_evt(view_t* view, ev_t ev)
{
  eh_list_t* eh = view->ehdata;
  if (ev.type == EV_TIME)
  {
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
          view_add(view, rowitem);
        }
        else
          eh->filled = 1;
      }
      else
      {
        view_t* head = mtvec_head(eh->items);
        view_t* tail = mtvec_tail(eh->items);

        // add items if needed

        if (head->frame.y > 0.0 - PRELOAD_DISTANCE)
        {
          view_t* cacheitem = mtvec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->head_index - 1);

          if (rowitem)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            mtvec_addatindex(eh->items, rowitem, 0);

            if (rowitem->parent == NULL) view_insert(view, rowitem, 0);
            view_set_frame(rowitem, (vframe_t){0, head->frame.y - rowitem->frame.h, rowitem->frame.w, rowitem->frame.h});

            eh->head_index -= 1;
          }
          else
            eh->filled = 1;
        }
        else
          eh->filled = 1;

        if (tail->frame.y + tail->frame.h < view->frame.h + PRELOAD_DISTANCE)
        {
          view_t* cacheitem = mtvec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->tail_index + 1);

          if (rowitem)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            VADD(eh->items, rowitem);

            if (rowitem->parent == NULL) view_add(view, rowitem);
            view_set_frame(rowitem, (vframe_t){0, tail->frame.y + tail->frame.h, rowitem->frame.w, rowitem->frame.h});

            eh->tail_index += 1;
          }
          else
            eh->filled &= 1;
        }
        else
          eh->filled &= 1;

        // remove items if needed

        if (head->frame.y + head->frame.h < 0.0 - PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, head);

          VREM(eh->items, head);
          //view_remove(view, head);

          eh->head_index += 1;
        }

        if (tail->frame.y > view->frame.h + PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, tail);

          VREM(eh->items, tail);
          //view_remove(view, tail);

          eh->tail_index -= 1;
        }
      }
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    view_t* sview;
    while ((sview = VNXT(view->views)))
    {
      vframe_t frame = sview->frame;
      frame.y += ev.dy;
      view_set_frame(sview, frame);
      eh->filled = 0;
    }
  }
}

void eh_list_del(void* p)
{
  eh_list_t* eh = (eh_list_t*)p;
  REL(eh->items);
}

void eh_list_add(view_t* view, view_t* (*row_generator)(view_t* listview, view_t* rowview, int index))
{
  printf("eh_list new\n");

  eh_list_t* eh     = mtmem_calloc(sizeof(eh_list_t), "eh_list", eh_list_del, NULL);
  eh->items         = VNEW();
  eh->cache         = VNEW();
  eh->row_generator = row_generator;

  view->ehdata = eh;
  view->eh     = eh_list_evt;
}

#endif
