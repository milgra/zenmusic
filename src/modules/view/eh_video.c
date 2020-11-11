#ifndef eh_video_h
#define ev_video_h

#include "player.c"
#include "view.c"
#include "wm_event.c"

void eh_video_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "ui_compositor.c"

typedef struct _eh_video_t
{
  uint32_t tex_ind;
} eh_video_t;

void eh_video_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    eh_video_t* eh = view->evt_han_data;

    player_draw_video(view->texture.page, view->frame.local.w, view->frame.local.h);
  }
}

void eh_video_add(view_t* view)
{
  eh_video_t* eh = mtmem_calloc(sizeof(eh_video_t), "eh_video", NULL, NULL);

  view->evt_han_data = eh;
  view->evt_han      = eh_video_evt;
}

#endif
