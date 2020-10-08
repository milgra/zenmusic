/*
 UI Connector Module for Zen Multimedia Desktop System

  events -> ui_connector -> ui_compositor -> gl_connector -> GPU
                 |
            ui_generator
 */

#ifndef ui_connector_h
#define ui_connector_h

void ui_connector_init(int, int);
void ui_connector_render();
void ui_connector_update(int x, int y);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtch.c"
#include "mtmap.c"
#include "mtvec.c"
#include "ui_compositor.c"
#include <pthread.h>
#include <unistd.h>

typedef struct _ui_rect_t
{
  char* id;
  char* desc;
  float x;
  float y;
  char inited;
  bm_t* bitmap;
} ui_rect_t;

ch_t* ui2rendch;
ch_t* rend2uich;
mtmap_t* uimap;
mtvec_t* uivec;
void* background_render_loop(void* mypointer);

void ui_connector_init(int width, int height)
{
  ui_compositor_init(width, height);

  uimap = MNEW();
  uivec = VNEW();

  bm_t* gr_bmp = bm_new(256, 256);
  bm_t* bl_bmp = bm_new(150, 70);

  bm_fill(gr_bmp, 0, 0, 256, 256, 0x00FF00FF);
  bm_fill(bl_bmp, 0, 0, 150, 70, 0x0000FFFF);

  ui_compositor_add("id1", 20.0, 20.0, 150.0, 70.0, bl_bmp);
  ui_compositor_add("id2", 200.0, 420.0, 256.0, 256.0, gr_bmp);

  pthread_t thread;

  ui2rendch = ch_new(10);
  rend2uich = ch_new(10);

  int success = pthread_create(&thread, NULL, background_render_loop, NULL);

  printf("thread created: %i\n", success);
}

void ui_connector_update(int x, int y)
{
}

void ui_connector_render()
{
  // check views without bitmap, request them from background thread

  // get rendered bitmaps, add them to views, update ui compositor
  char* data = (char*)ch_recv(rend2uich);
  if (data)
    printf("render, recv data %s\n", data);

  // update views in compositor

  ui_compositor_render();
}

void* background_render_loop()
{

  while (1)
  {
    ch_send(rend2uich, "anyad");
    sleep(5);
  }
}

#endif
