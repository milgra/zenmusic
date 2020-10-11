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
void ui_connector_rem(view_t* view);
void ui_connector_resize(float width, float height);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtchannel.c"
#include "mtmap.c"
#include "mtvector.c"
#include "ui_compositor.c"
#include "view.c"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

mtmap_t*  uim;
mtvec_t*  uiv;
mtch_t*   uich;
pthread_t uibgth;
void*     ui_connector_workloop(void* mypointer);

int ui_connector_init(int width, int height)
{
  ui_compositor_init(width, height);

  uim  = MNEW();
  uiv  = VNEW();
  uich = mtch_new(10);

  int success = pthread_create(&uibgth,
                               NULL,
                               ui_connector_workloop,
                               NULL);

  return success;
}

void ui_connector_reset()
{
  ui_compositor_reset();
  mtvec_reset(uiv);
  mtmap_reset(uim);
}

void ui_connector_add(view_t* view)
{
  printf("add %s\n", view->id);
  VADD(uiv, view);
  view->attached = 1;
}

void ui_connector_rem(view_t* view)
{
  VREM(uiv, view);
  view->attached = 0;
}

void ui_connector_render()
{
  view_t* view;

  while ((view = VNXT(uiv)))
  {
    if (view->bmp_state == 0) /* send unrendered views to renderer thread */
    {
      view->bmp_state = 1; /* pending */
      mtch_send(uich, view);
    }
    if (view->bmp_state == 2) /* add rendered views to compositor */
    {
      view->bmp_state   = 3; /* added to compositor */
      view->bmp_changed = 0;
      ui_compositor_add(view->id,
                        view->frame.x,
                        view->frame.y,
                        view->frame.w,
                        view->frame.h,
                        view->bmp);
    }
    if (view->frame_changed) /* update dimension if needed */
    {
      ui_compositor_upd(view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h, NULL);
      view->frame_changed = 0;
    }
    if (view->bmp_changed) /* update bitmap if needed */
    {
      ui_compositor_upd(view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h, view->bmp);
      view->bmp_changed = 0;
    }
  }

  ui_compositor_render();
}

void* ui_connector_workloop()
{
  struct timespec ts;
  view_t*         view;

  ts.tv_sec  = 0;
  ts.tv_nsec = 16000000;

  while (1)
  {
    if ((view = mtch_recv(uich)))
    {
      printf("generating bmp for %s\n", view->id);
      view_tex(view);
    }
    nanosleep(&ts, &ts);
  }
}

void ui_connector_resize(float width, float height)
{
  ui_compositor_resize(width, height);
}

#endif
