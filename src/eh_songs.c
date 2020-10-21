#ifndef eh_songs_h
#define eh_songs_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_songs_t
{
  float    headpos;
  mtvec_t* items;
} eh_songs_t;

void eh_songs_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtstring.c"
#include "tg_text.c"
#include <math.h>

void eh_songs_evt(view_t* view, ev_t ev)
{
  eh_songs_t* eh = view->ehdata;
  if (ev.type == EV_SCROLL)
  {
    eh->headpos += ev.dy;
    if (eh->headpos < 0.0) eh->headpos = 0.0;

    view_t* sview;
    while ((sview = VNXT(view->views)))
    {
      vframe_t frame = sview->frame;
      frame.y        = round(eh->headpos);
      view_set_frame(sview, frame);
    }

    // generate new views if needed
    view_t* head = mtvec_head(view->views);
    view_t* tail = mtvec_tail(view->views);

    // list not empty or not one row
    if (head != tail)
    {
      if (head->frame.y > 0.0) printf("needs head rows\n");
      if (head->frame.y < view->frame.x + view->frame.h) printf("needs tail rows\n");
    }
  }
}

void eh_songs_del(void* p)
{
  eh_songs_t* eh = (eh_songs_t*)p;
  REL(eh->items);
}

void eh_songs_add(view_t* view)
{
  printf("eh_songs new\n");

  eh_songs_t* eh = mtmem_alloc(sizeof(eh_songs_t), "eh_songs", eh_songs_del, NULL);
  eh->items      = VNEW();
  eh->headpos    = 0;

  view_t* item0 = view_new("list_item0", (vframe_t){0, 0, 350, 50}, 0);
  view_t* item1 = view_new("list_item1", (vframe_t){0, 50, 350, 50}, 0);

  tg_text_add(item0, 0xFFFFFFFF, 0x000000FF, "ITEM0");
  tg_text_add(item1, 0xFFFFFFFF, 0x000000FF, "ITEM1");

  view_add(view, item0);
  view_add(view, item1);

  view->ehdata = eh;
  view->eh     = eh_songs_evt;
}

#endif
