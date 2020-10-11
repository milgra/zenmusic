#ifndef evthan_music_list_h
#define evthan_music_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _mlist_t
{
  float    headpos;
  mtvec_t* items;
} mlist_t;

void musiclist_new(view_t* view, void* data);
void musiclist_event(view_t* view, ev_t ev);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtstring.c"
#include "tex/text.c"
#include <math.h>

void musiclist_del(void* p)
{
  mlist_t* list = (mlist_t*)p;
  REL(list->items);
}

void musiclist_new(view_t* view, void* data)
{
  printf("musiclist new\n");

  mlist_t* mlist = mtmem_alloc(sizeof(mlist_t), "musiclist", musiclist_del, NULL);
  mlist->items   = VNEW();
  mlist->headpos = 0;

  view_setdata(view, mlist);

  view_t* item0 = view_new("list_item0",
                           (vframe_t){0, 0, 350, 50},
                           NULL,
                           text_gen,
                           NULL,
                           NULL);
  view_t* item1 = view_new("list_item1",
                           (vframe_t){0, 50, 350, 50},
                           NULL,
                           text_gen,
                           NULL,
                           NULL);
  view_add(view, item0);
  view_add(view, item1);
}

void musiclist_event(view_t* view, ev_t ev)
{
  mlist_t* list = view->data;
  if (ev.type == EV_TIME)
  {
  }
  else if (ev.type == EV_SCROLL)
  {
    list->headpos += ev.dy;
    if (list->headpos < 0.0) list->headpos = 0.0;

    view_t* sview;
    while ((sview = VNXT(view->views)))
    {
      vframe_t frame = sview->frame;
      frame.y        = round(list->headpos);
      view_setframe(sview, frame);
    }
  }
  else if (ev.type == EV_MMOVE && ev.drag)
  {
    view_t* sview;
    while ((sview = VNXT(view->views)))
    {
      vframe_t frame = sview->frame;
      frame.y += (float)ev.dy;
      view_setframe(sview, frame);
    }
  }
}

#endif
