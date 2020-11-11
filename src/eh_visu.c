#ifndef eh_visu_h
#define ev_visu_h

#include "player.c"
#include "view.c"
#include "wm_event.c"

typedef enum _audio_vis_t
{
  VIS_NONE,
  VIS_WAVE,
  VIS_RDFT
} audio_vis_t;

audio_vis_t audio_vis = VIS_WAVE;

void eh_visu_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "ui_compositor.c"

typedef struct _eh_visu_t
{
  uint32_t tex_ind;
  int      vis_type;
} eh_visu_t;

void eh_visu_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    eh_visu_t* eh = view->evt_han_data;

    if (audio_vis == VIS_WAVE)
    {
      player_draw_waves(view->texture.index, 0, view->texture.bitmap);
    }
    else if (audio_vis == VIS_RDFT)
    {
      player_draw_rdft(view->texture.index, 0, view->texture.bitmap);
    }
  }
}

void eh_visu_add(view_t* view)
{
  eh_visu_t* eh = mtmem_calloc(sizeof(eh_visu_t), "eh_visu", NULL, NULL);

  view->evt_han_data = eh;
  view->evt_han      = eh_visu_evt;
}

#endif
