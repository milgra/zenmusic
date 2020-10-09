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
void ui_connector_render();
void ui_connector_add(view_t* view);
void ui_connector_del(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtch.c"
#include "mtmap.c"
#include "mtvec.c"
#include "ui_compositor.c"
#include "view.c"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

mtmap_t*  uim;
mtvec_t*  uiv;
ch_t*     uich;
pthread_t uibgth;
void*     ui_connector_workloop(void* mypointer);

int ui_connector_init(int width, int height)
{
  ui_compositor_init(width, height);

  uim  = MNEW();
  uiv  = VNEW();
  uich = ch_new(10);

  int success = pthread_create(&uibgth,
                               NULL,
                               ui_connector_workloop,
                               NULL);

  return success;
}

void ui_connector_add(view_t* view)
{
  VADD(uiv, view);
}

void ui_connector_del(view_t* view)
{
  VREM(uiv, view);
}

void ui_connector_render()
{
  view_t* view;

  while ((view = VNXT(uiv)))
  {
    if (view->bmp_state == 0) // send unrendered views to renderer thread
    {
      view->bmp_state = 1; // pending
      ch_send(uich, view);
    }
    if (view->bmp_state == 2) // add rendered views to compositor
    {
      view->bmp_state   = 3; // added to compositor
      view->bmp_changed = 0;
      ui_compositor_add(view->id,
                        view->frame.x,
                        view->frame.y,
                        view->frame.z,
                        view->frame.w,
                        view->bmp);
    }
    if (view->frame_changed) // update dimension if needed
    {
      ui_compositor_upd(view->id, view->frame.x, view->frame.y, view->frame.z, view->frame.w, NULL);
      view->frame_changed = 0;
    }
    if (view->bmp_changed) // update bitmap if needed
    {
      ui_compositor_upd(view->id, view->frame.x, view->frame.y, view->frame.z, view->frame.w, view->bmp);
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
  ts.tv_nsec = 15000000;

  while (1)
  {
    if ((view = ch_recv(uich)))
    {
      printf("generating bmp for %s\n", view->id);
      view_tex(view);
    }
    nanosleep(&ts, &ts);
  }
}

#endif
