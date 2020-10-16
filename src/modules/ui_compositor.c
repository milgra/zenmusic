/*
  UI Compositor Module for Zen Multimedia Desktop System
  Places incoming bitmaps in texture maps, points in floatbuffers, renders rects on demand.

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "mtbitmap.c"

void ui_compositor_init(int, int);
void ui_compositor_render();
void ui_compositor_reset();
void ui_compositor_add(char* id, int x, int y, int w, int h, bm_t* bmp);
void ui_compositor_upd(char* id, int x, int y, int w, int h, bm_t* bmp);
void ui_compositor_rem(char* id);
void ui_compositor_resize(float width, float height);

typedef struct _crect_t
{
  float data[24];
  char* id;
} crect_t;

crect_t* crect_new(char* id, float x, float y, float w, float h, float tx, float ty, float tz, float tw);
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_dim(crect_t* rect, float x, float y, float w, float h);
void     crect_set_tex(crect_t* rect, float tx, float ty, float tz, float tw);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_connector.c"
#include "gl_floatbuffer.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtmath4.c"
#include "mttexmap.c"
#include "mtvector.c"

fb_t*    fb;    // float buffer
tm_t*    tm;    // texture map
mtvec_t* rectv; // rectangle vector
mtmap_t* rectm; // rectangle map

void ui_compositor_init(int width, int height)
{
  gl_init(width, height);

  fb    = fb_new();
  tm    = tm_new();
  rectv = VNEW();
  rectm = MNEW();
}

void ui_compositor_reset()
{
  fb_reset(fb);
  tm_reset(tm);
  mtvec_reset(rectv);
  mtmap_reset(rectm);
}

void ui_compositor_render()
{
  crect_t* rect;

  fb_reset(fb);

  while ((rect = VNXT(rectv)))
    fb_add(fb, rect->data, 24);

  gl_render(fb, tm->bm);
}

void ui_compositor_add(char* id, int x, int y, int w, int h, bm_t* bmp)
{
  v4_t     texc;
  crect_t* rect;

  if (bmp)
  {
    tm_put(tm, id, bmp);

    texc = tm_get(tm, id);
    rect = crect_new(id, x, y, w, h, texc.x, texc.y, texc.z, texc.w);
  }
  else
  {
    rect = crect_new(id, x, y, w, h, 0.0, 0.0, 1.0, 1.0);
  }

  VADD(rectv, rect);
  MPUT(rectm, id, rect);
}

void ui_compositor_upd(char* id, int x, int y, int w, int h, bm_t* bmp)
{
  crect_t* rect;

  if ((rect = MGET(rectm, id)))
  {
    crect_set_dim(rect, x, y, w, h);
    if (bmp) tm_upd(tm, id, bmp);
  }
}

void ui_compositor_rem(char* id)
{
  crect_t* rect;

  rect = mtmap_get(rectm, id);
  MDEL(rectm, id);
  VREM(rectv, rect);
}

void ui_compositor_resize(float width, float height)
{
  gl_resize(width, height);
}

//
// Compositor Rect
//

crect_t* crect_new(char* id,
                   float x,
                   float y,
                   float w,
                   float h,
                   float tx,
                   float ty,
                   float tz,
                   float tw)
{
  crect_t* r = mtmem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  r->id = mtcstr_fromcstring(id);

  r->data[0] = x;
  r->data[1] = y;
  r->data[2] = tx;
  r->data[3] = ty;

  r->data[4] = x + w;
  r->data[5] = y + h;
  r->data[6] = tz;
  r->data[7] = tw;

  r->data[8]  = x;
  r->data[9]  = y + h;
  r->data[10] = tx;
  r->data[11] = tw;

  r->data[12] = x + w;
  r->data[13] = y;
  r->data[14] = tz;
  r->data[15] = ty;

  r->data[16] = x;
  r->data[17] = y;
  r->data[18] = tx;
  r->data[19] = ty;

  r->data[20] = x + w;
  r->data[21] = y + h;
  r->data[22] = tz;
  r->data[23] = tw;

  return r;
}

void crect_del(void* pointer)
{
  crect_t* r = (crect_t*)pointer;
  REL(r->id);
}

void crect_set_dim(crect_t* r, float x, float y, float w, float h)
{
  r->data[0] = x;
  r->data[1] = y;

  r->data[4] = x + w;
  r->data[5] = y + h;

  r->data[8] = x;
  r->data[9] = y + h;

  r->data[12] = x + w;
  r->data[13] = y;

  r->data[16] = x;
  r->data[17] = y;

  r->data[20] = x + w;
  r->data[21] = y + h;
}

void crect_set_tex(crect_t* r, float tx, float ty, float tz, float tw)
{
  r->data[2] = tx;
  r->data[3] = ty;

  r->data[6] = tz;
  r->data[7] = tw;

  r->data[10] = tx;
  r->data[11] = tw;

  r->data[14] = tz;
  r->data[15] = ty;

  r->data[18] = tx;
  r->data[19] = ty;

  r->data[22] = tz;
  r->data[23] = tw;
}

void crect_desc(crect_t* r)
{
  for (int index = 0; index < 24; index++)
  {
    if (index % 4 == 0) printf("\n");
    printf("%f ", r->data[index]);
  }
  printf("\n");
}

#endif
