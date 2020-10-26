#ifndef eh_songs_h
#define eh_songs_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_songs_t
{
  mtvec_t* items;
  mtvec_t* files;

  int head_index;
  int tail_index;
} eh_songs_t;

void eh_songs_add(view_t* view, mtvec_t* files);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_text.c"
#include "ui_manager.c"
#include <math.h>

void eh_songs_evt(view_t* view, ev_t ev)
{
  eh_songs_t* eh = view->ehdata;
  if (ev.type == EV_TIME)
  {
    if (eh->items->length == 0 && eh->files->length > 0)
    {
      // create first item
      char idbuffer[100];
      int  i = 0;
      snprintf(idbuffer, 100, "list_item%i", i);
      view_t*  item  = view_new(idbuffer, (vframe_t){0, i * 40, 900, 41}, 0);
      uint32_t color = (i % 2 == 0) ? 0xEFEFEFFF : 0xDEDEDEFF;
      tg_text_add(item, color, 0x000000FF, eh->files->data[i]);
      view_add(view, item);
      VADD(eh->items, item);

      eh->head_index = 0;
      eh->tail_index = 0;
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
    }

    if (eh->items->length > 0)
    {
      view_t* head = mtvec_head(eh->items);
      view_t* tail = mtvec_tail(eh->items);

      if (head->frame.y > 0.0)
      {
        // add new head
        printf("adding head\n");

        if (eh->head_index > 0)
        {
          char idbuffer[100];
          int  i = eh->head_index - 1;
          snprintf(idbuffer, 100, "list_item%i", i);
          view_t*  item  = view_new(idbuffer, (vframe_t){0, head->frame.y - 41, 900, 41}, 0);
          uint32_t color = (i % 2 == 0) ? 0xEFEFEFFF : 0xDEDEDEFF;
          tg_text_add(item, color, 0x000000FF, eh->files->data[i]);
          view_insert(view, item, 0);
          mtvec_addatindex(eh->items, item, 0);
          eh->head_index = i;
          //REL(item);

          ui_manager_add(item);
        }
      }
      else if (head->frame.y + head->frame.h < 0.0)
      {
        // remove head
        printf("adding head\n");
        ui_manager_remove(head);

        VREM(eh->items, head);
        view_remove(view, head);
        eh->head_index += 1;
      }

      if (tail->frame.y + tail->frame.h < view->frame.h)
      {
        // add new tail
        if (eh->files->length > eh->tail_index)
        {
          char idbuffer[100];
          int  i = eh->tail_index + 1;
          snprintf(idbuffer, 100, "list_item%i", i);
          view_t*  item  = view_new(idbuffer, (vframe_t){0, tail->frame.y + tail->frame.h, 900, 41}, 0);
          uint32_t color = (i % 2 == 0) ? 0xEFEFEFFF : 0xDEDEDEFF;
          tg_text_add(item, color, 0x000000FF, eh->files->data[i]);
          view_add(view, item);
          VADD(eh->items, item);

          ui_manager_add(item);

          eh->tail_index = i;
          //REL(item);
        }
      }
      else if (tail->frame.y > view->frame.h)
      {
        // remove tail
        ui_manager_remove(tail);

        VREM(eh->items, tail);
        view_remove(view, tail);
        eh->tail_index -= 1;
      }
    }
  }
}

void eh_songs_del(void* p)
{
  eh_songs_t* eh = (eh_songs_t*)p;
  REL(eh->items);
}

void eh_songs_add(view_t* view, mtvec_t* files)
{
  printf("eh_songs new\n");

  eh_songs_t* eh = mtmem_alloc(sizeof(eh_songs_t), "eh_songs", eh_songs_del, NULL);
  eh->files      = files;
  eh->items      = VNEW();

  view->ehdata = eh;
  view->eh     = eh_songs_evt;
}

#endif
