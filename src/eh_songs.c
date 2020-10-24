#ifndef eh_songs_h
#define eh_songs_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_songs_t
{
  float    headpos;
  mtvec_t* items;
  mtvec_t* files;
} eh_songs_t;

void eh_songs_add(view_t* view, mtvec_t* files);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_text.c"
#include <math.h>

void eh_songs_evt(view_t* view, ev_t ev)
{
  eh_songs_t* eh = view->ehdata;
  if (ev.type == EV_TIME)
  {
    if (eh->items->length == 0 && eh->files->length > 0)
    {
      for (int i = 0; i < 20; i++)
      {
        char idbuffer[100];
        snprintf(idbuffer, 100, "list_item%i", i);
        view_t* item = view_new(idbuffer, (vframe_t){0, i * 50.0, 900, 50}, 0);

        tg_text_add(item, 0xFFFFFFFF, 0x000000FF, eh->files->data[i]);
        view_add(view, item);

        VADD(eh->items, item);
      }
    }
  }
  else if (ev.type == EV_SCROLL)
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

    if (eh->files->length > 0)
    {

      // generate new views if needed
      view_t* head = mtvec_head(view->views);
      view_t* tail = mtvec_tail(view->views);

      // list not empty or not one row
      if (head != tail)
      {
        //if (head->frame.y > 0.0) printf("needs head rows\n");
        //if (head->frame.y < view->frame.x + view->frame.h) printf("needs tail rows\n");
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
  eh->headpos    = 0;

  view->ehdata = eh;
  view->eh     = eh_songs_evt;
}

#endif
