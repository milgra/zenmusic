#ifndef evthan_music_list_h
#define evthan_music_list_h

#include "view.c"
#include "wm_event.c"

void evthan_music_list(view_t* view, ev_t event);

#endif

#if __INCLUDE_LEVEL__ == 0

void evthan_music_list(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE && ev.drag)
  {
    printf("evthan_music_list");
  }
}

#endif
