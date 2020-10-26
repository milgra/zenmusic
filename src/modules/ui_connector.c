/*
  UI Connector Module for Zen Multimedia Desktop System 
  Monitors views for changes, renders their content in the background, updates compositor state based on view state.

  views  ->
  events -> ui_connector -> ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_connector_h
#define ui_connector_h

#include "view.c"

int  ui_connector_init(int, int);
void ui_connector_reset();
void ui_connector_render();
void ui_connector_add(view_t* view);
void ui_connector_remove(view_t* view);
void ui_connector_set_index(view_t* view);
void ui_connector_resize(float width, float height);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtchannel.c"
#include "mtmap.c"
#include "mtvector.c"
#include "ui_compositor.c"
#include "view.c"
#include <SDL.h>
#include <unistd.h>

mtmap_t*  uim;
mtvec_t*  uiv;
mtch_t*   uich;
pthread_t uibgth;
int       ui_connector_workloop(void* mypointer);

int ui_connector_init(int width, int height)
{
  ui_compositor_init(width, height);

  uim  = MNEW();
  uiv  = VNEW();
  uich = mtch_new(10);

  // TODO SDL_CreateThread
  SDL_Thread* thread = SDL_CreateThread(ui_connector_workloop,
                                        "connector",
                                        NULL);

  return (thread != NULL);
}

void ui_connector_reset()
{
  ui_compositor_reset();
  mtvec_reset(uiv);
  mtmap_reset(uim);
}

void ui_connector_add(view_t* view)
{
  VADD(uiv, view);
  ui_compositor_add(view->id,
                    view->index,
                    view->tex_channel,
                    view->frame.x,
                    view->frame.y,
                    view->frame.w,
                    view->frame.h);
}

void ui_connector_remove(view_t* view)
{
  VREM(uiv, view);
  ui_compositor_rem(view->id);
}

void ui_connector_set_index(view_t* view)
{
  ui_compositor_set_index(view->id, view->index);
}

void ui_connector_render()
{
  view_t* view;

  while ((view = VNXT(uiv)))
  {
    if (view->tex_state == TS_BLANK) /* send unrendered views to renderer thread */
    {
      if (mtch_send(uich, view))
      {
        view->tex_state = TS_PENDING;
      }
    }
    if (view->frame_changed) /* update dimension if needed */
    {
      ui_compositor_set_frame(view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h);
      view->frame_changed = 0;
    }
    if (view->tex_changed) /* update bitmap if needed */
    {
      ui_compositor_set_texture(view->id, view->tex);
      view->tex_changed = 0;
    }
  }

  ui_compositor_render();
}

int ui_connector_workloop()
{
  view_t* view;

  while (1)
  {
    if ((view = mtch_recv(uich)))
    {
      printf("generating bmp for %s\n", view->id);
      view_gen_texture(view);
    }
    SDL_Delay(16);
  }
}

void ui_connector_resize(float width, float height)
{
  ui_compositor_resize(width, height);
}

#endif
