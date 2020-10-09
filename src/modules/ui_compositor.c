/*
  UI Compositor Module for Zen Multimedia Desktop System
  Places incoming bitmaps in texture maps, renders rects on demand.

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "mtbm.c"

void ui_compositor_init(int, int);
void ui_compositor_render();
void ui_compositor_add(char* id, int x, int y, int w, int h, bm_t* bmp);
void ui_compositor_upd(char* id, int x, int y, int w, int h, bm_t* bmp);
void ui_compositor_rem(char* id);

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
#include "math4.c"
#include "mtbm.c"
#include "mtcstr.c"
#include "mtfb.c"
#include "mtmap.c"
#include "mttm.c"
#include "mtvec.c"

fb_t*    flt_buf;
tm_t*    tex_map;
mtvec_t* rectv;
mtmap_t* rectm;

void ui_compositor_init(int width, int height)
{
  gl_init(width, height);

  flt_buf = fb_new();
  tex_map = tm_new();
  rectv   = VNEW();
  rectm   = MNEW();
}

void ui_compositor_render()
{
  crect_t* rect;

  fb_reset(flt_buf);

  while ((rect = VNXT(rectv)))
    fb_add(flt_buf, rect->data, 24);

  gl_render(flt_buf, tex_map->bm);
}

void ui_compositor_add(char* id, int x, int y, int w, int h, bm_t* bmp)
{
  v4_t     texc;
  crect_t* rect;

  tm_put(tex_map, id, bmp);

  texc = tm_get(tex_map, id);
  rect = crect_new(id, x, y, w, h, texc.x, texc.y, texc.z, texc.w);

  VADD(rectv, rect);
  MPUT(rectm, id, rect);
}

void ui_compositor_upd(char* id, int x, int y, int w, int h, bm_t* bmp)
{
  crect_t* rect;

  if ((rect = MGET(rectm, id)))
  {
    crect_set_dim(rect, x, y, w, h);
    if (bmp) tm_upd(tex_map, id, bmp);
  }
}

void ui_compositor_rem(char* id)
{
  crect_t* rect;

  rect = mtmap_get(rectm, id);
  MDEL(rectm, id);
  VREM(rectv, rect);
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
  crect_t* r = mtmem_calloc(sizeof(crect_t), crect_del);

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