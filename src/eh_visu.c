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

void eh_visu_add(view_t* view, int channel);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "ui_compositor.c"

typedef struct _eh_visu_t
{
  int vis_type;
  int channel;
} eh_visu_t;

void eh_visu_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME && view->texture.bitmap != NULL)
  {
    eh_visu_t* eh = view->evt_han_data;

    if (view->texture.bitmap == NULL)
    {
      bm_t* bmp = bm_new(view->frame.local.w, view->frame.local.h);
      bm_fill(bmp,
              0,
              0,
              view->frame.local.w,
              view->frame.local.h,
              0x000000FF);

      char idbuffer[100] = {0};
      snprintf(idbuffer, 20, "visu %i", eh->channel);

      view_set_texture(view, bmp, idbuffer);
    }

    if (eh->vis_type == VIS_WAVE)
    {
      player_draw_waves(view->texture.page, eh->channel, view->texture.bitmap);
    }
    else if (eh->vis_type == VIS_RDFT)
    {
      player_draw_rdft(view->texture.page, eh->channel, view->texture.bitmap);
    }
    // TODO only update when really changed
    view->texture.changed = 1;
  }
}

void eh_visu_add(view_t* view, int channel)
{
  eh_visu_t* eh = mtmem_calloc(sizeof(eh_visu_t), "eh_visu", NULL, NULL);

  eh->channel  = channel;
  eh->vis_type = VIS_WAVE;

  view->evt_han_data = eh;
  view->evt_han      = eh_visu_evt;
}

#endif
