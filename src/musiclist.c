#ifndef evthan_music_list_h
#define evthan_music_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _mlist_t
{
  float    headpos;
  float    speed;
  float    slowdown;
  uint32_t scroll;
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

  mlist_t* mlist  = mtmem_alloc(sizeof(mlist_t), "musiclist", musiclist_del, NULL);
  mlist->items    = VNEW();
  mlist->headpos  = 0;
  mlist->slowdown = 0.95;

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
    float ratio = (float)ev.dtime / 16.0;
    list->headpos += list->speed;
    list->speed *= list->slowdown;

    view_t* sview;
    while ((sview = VNXT(view->views)))
    {
      vframe_t frame = sview->frame;
      frame.y += round(list->headpos);
      view_setframe(sview, frame);
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    uint32_t delta = ev.time - list->scroll;
    list->scroll   = ev.time;
    list->slowdown = delta < 10 ? 0.5 : 0.4;
    float multi    = delta < 10 ? 2.0 : 8.0;
    list->speed += ev.dy;
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
