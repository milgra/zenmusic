#ifndef eh_list_h
#define eh_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_list_t
{
  mtvec_t* items;
  mtvec_t* cache;

  int  index;
  int  head_index;
  int  tail_index;
  char filled;

  char (*row_generator)(view_t* listview, view_t* rowview, int index); /* event handler for view */
} eh_list_t;

void eh_list_add(view_t* view, char (*row_generator)(view_t* listview, view_t* rowview, int index));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_text.c"
#include "ui_manager.c"
#include <math.h>

view_t* eh_list_gen_item(view_t* view)
{
  eh_list_t* eh = view->ehdata;
  view_t*    item;

  if (eh->cache->length > 0)
  {
    item = eh->cache->data[0];
  }
  else
  {
    char idbuffer[100];
    snprintf(idbuffer, 100, "list_item%i", eh->index++);
    item = view_new(idbuffer, (vframe_t){0, 0, 0, 0}, 0);
    VADD(eh->cache, item);
  }
  return item;
}

void eh_list_cache_item(view_t* view, view_t* rowitem)
{
  eh_list_t* eh = view->ehdata;
  VADD(eh->cache, rowitem);

  rowitem->tex_state = TS_BLANK;
  rowitem->eh        = NULL;
  rowitem->tg        = NULL;
  rowitem->ehdata    = NULL;
  rowitem->tgdata    = NULL;
}

void eh_list_evt(view_t* view, ev_t ev)
{
  eh_list_t* eh = view->ehdata;
  if (ev.type == EV_TIME)
  {
    while (eh->filled == 0)
    {
      if (eh->items->length == 0)
      {
        view_t* rowitem = eh_list_gen_item(view);
        char    success = (*eh->row_generator)(view, rowitem, 0);

        if (success)
        {
          VREM(eh->cache, rowitem);
          view_add(view, rowitem);
          VADD(eh->items, rowitem);
          ui_manager_add(rowitem);
        }
        else
          eh->filled = 1;
      }
      else
      {
        view_t* head = mtvec_head(eh->items);
        view_t* tail = mtvec_tail(eh->items);

        // add items if needed

        if (head->frame.y > 0.0)
        {
          view_t* rowitem = eh_list_gen_item(view);
          char    success = (*eh->row_generator)(view, rowitem, eh->head_index - 1);

          if (success)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            mtvec_addatindex(eh->items, rowitem, 0);

            view_insert(view, rowitem, 0);
            view_set_frame(rowitem, (vframe_t){0, head->frame.y - rowitem->frame.h, rowitem->frame.w, rowitem->frame.h});

            eh->head_index -= 1;
            ui_manager_add(rowitem);
          }
          else
            eh->filled = 1;
        }
        else
          eh->filled = 1;

        if (tail->frame.y + tail->frame.h < view->frame.h)
        {
          view_t* rowitem = eh_list_gen_item(view);
          char    success = (*eh->row_generator)(view, rowitem, eh->tail_index + 1);

          if (success)
          {
            eh->filled = 0; // there is probably more to come

            VREM(eh->cache, rowitem);
            VADD(eh->items, rowitem);

            view_add(view, rowitem);
            view_set_frame(rowitem, (vframe_t){0, tail->frame.y + tail->frame.h, rowitem->frame.w, rowitem->frame.h});

            eh->tail_index += 1;
            ui_manager_add(rowitem);
          }
          else
            eh->filled &= 1;
        }
        else
          eh->filled &= 1;

        // remove items if needed

        if (head->frame.y + head->frame.h < 0.0 && eh->items->length > 1)
        {
          eh_list_cache_item(view, head);

          VREM(eh->items, head);
          ui_manager_remove(head);
          view_remove(view, head);

          eh->head_index += 1;
        }

        if (tail->frame.y > view->frame.h && eh->items->length > 1)
        {
          eh_list_cache_item(view, tail);

          VREM(eh->items, tail);
          ui_manager_remove(tail);
          view_remove(view, tail);

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

void eh_list_add(view_t* view, char (*row_generator)(view_t* listview, view_t* rowview, int index))
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
