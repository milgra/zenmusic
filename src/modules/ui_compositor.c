/*
  UI Compositor Module for Zen Multimedia Desktop System
  Places incoming bitmaps in texture maps, incoming rectangles and texture map positions in floatbuffers, renders rects on demand.

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "gl_connector.c"
#include "mtbitmap.c"

void ui_compositor_init(int, int);
void ui_compositor_render();
void ui_compositor_reset();
void ui_compositor_add(char* id, uint32_t index, int channel, int x, int y, int w, int h, float tx, float ty, float tz, float tw, char shadow, char blurred);
void ui_compositor_rem(char* id);
void ui_compositor_set_index(char* id, uint32_t index);
void ui_compositor_set_frame(char* id, int x, int y, int w, int h);
void ui_compositor_set_texture(char* id, bm_t* bmp);
void ui_compositor_resize(int width, int height);

typedef struct _crect_t
{
  char*    id;
  float    data[30];
  uint32_t index;
  char     shadow;
  char     blur;
  region_t region;
} crect_t;

crect_t* crect_new(char* id, uint32_t index, uint32_t channel, float x, float y, float w, float h, float tx, float ty, float tz, float tw);
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_index(crect_t* rest, uint32_t index);
void     crect_set_frame(crect_t* rect, float x, float y, float w, float h);
void     crect_set_texture(crect_t* rect, float tx, float ty, float tz, float tw);

#endif

#if __INCLUDE_LEVEL__ == 0

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
int      comp_width;
int      comp_height;

void ui_compositor_init(int width, int height)
{
  gl_init(width, height);

  fb    = fb_new();
  tm    = tm_new(4096, 4096);
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
  gl_update_vertexes(fb);
  gl_update_textures(tm->bm);
  gl_clear_framebuffer(0, 0.0, 0.0, 0.0, 1.0);
  //gl_clear_framebuffer(3, 0.0, 0.0, 0.0, 1.0);

  region_t reg_full = {0, 0, comp_width, comp_height};
  region_t reg_half = {0, 0, comp_width / 2, comp_height / 2};

  crect_t* rect;
  int      last  = 0;
  int      index = 0;
  for (index = 0; index < rectv->length; index++)
  {
    rect = rectv->data[index];
    if (rect->shadow || rect->blur)
    {
      // render rects so far with simple texture renderer to offscreen buffer
      gl_draw_vertexes_in_framebuffer(3, last * 6, index * 6, reg_full, reg_full, SH_TEXTURE);

      last = index;

      if (rect->shadow)
      {
        // render current view with black color to an offscreen buffer
        gl_clear_framebuffer(4, 0.0, 0.0, 0.0, 0.0);
        gl_clear_framebuffer(5, 0.0, 0.0, 0.0, 0.0);
        gl_draw_vertexes_in_framebuffer(4, index * 6, (index + 1) * 6, reg_full, reg_half, SH_COLOR);
        // blur offscreen buffer for soft shadows
        gl_draw_framebuffer_in_framebuffer(4, 5, reg_half, reg_half, ((region_t){0}), SH_BLUR);
        // draw offscreen buffer on final buffer
        gl_draw_framebuffer_in_framebuffer(5, 3, reg_half, reg_full, ((region_t){0}), SH_TEXTURE);
      }

      if (rect->blur)
      {
        // render current state with texture shader to an offscreen buffer
        gl_clear_framebuffer(6, 0.0, 0.0, 0.0, 0.0);
        gl_clear_framebuffer(5, 0.0, 0.0, 0.0, 0.0);
        // shrink current framebuffer for blur
        gl_draw_framebuffer_in_framebuffer(3, 6, reg_full, reg_half, ((region_t){0}), SH_TEXTURE);

        // blur offscreen buffer for soft shadows
        gl_draw_framebuffer_in_framebuffer(6, 5, reg_half, reg_half, ((region_t){0}), SH_BLUR);
        gl_draw_framebuffer_in_framebuffer(5, 6, reg_half, reg_half, ((region_t){0}), SH_BLUR);

        // draw blurred buffer on final buffer inside the view
        gl_draw_framebuffer_in_framebuffer(6, 3, reg_half, reg_full, rect->region, SH_TEXTURE);

        // skip drawing actual rect when blur
        last++;
        index++;
      }
    }
  }

  if (last < index)
  {
    // render remaining
    gl_draw_vertexes_in_framebuffer(3, last * 6, index * 6, reg_full, reg_full, SH_TEXTURE);
  }

  // finally draw offscreen buffer to screen buffer
  gl_draw_framebuffer_in_framebuffer(3, 0, reg_full, reg_full, ((region_t){0}), SH_TEXTURE);
}

void ui_compositor_update()
{
  crect_t* rect;

  fb_reset(fb);

  while ((rect = VNXT(rectv)))
  {
    /* if (strcmp(rect->id, "chessview") == 0) */
    /* { */
    /*   printf("adding vertexs for %s index %i\n", rect->id, rect->index); */
    /*   crect_desc(rect); */
    /* } */
    fb_add(fb, rect->data, 30);
  }
}

void ui_compositor_add(char*    id,
                       uint32_t index,
                       int      channel,
                       int      x,
                       int      y,
                       int      w,
                       int      h,
                       float    tx,
                       float    ty,
                       float    tz,
                       float    tw,
                       char     shadow,
                       char     blur)
{
  // printf("ui_compositor_add %s index %i channel %i %i %i %i %i\n", id, index, channel, x, y, w, h);
  crect_t* rect = crect_new(id, index, channel, x, y, w, h, tx, ty, tz, tw);
  rect->shadow  = shadow;
  rect->blur    = blur;

  VADD(rectv, rect);
  MPUT(rectm, id, rect);

  ui_compositor_update();
}

void ui_compositor_rem(char* id)
{
  // printf("ui_compositor_rem %s\n", id);
  crect_t* rect;

  rect = mtmap_get(rectm, id);
  MDEL(rectm, id);
  VREM(rectv, rect);

  ui_compositor_update();
}

void ui_compositor_set_index(char* id, uint32_t index)
{
  // printf("ui_compositor_set_index %s %i\n", id, index);

  crect_t* rect;

  if ((rect = MGET(rectm, id)))
  {
    uint32_t oldindex = mtvec_indexofdata(rectv, rect);
    // printf("%s old index %i new index %i\n", id, oldindex, index);
    if (oldindex != index)
    {
      mtvec_rem(rectv, rect);
      mtvec_addatindex(rectv, rect, index);
      rect->index = index;
      ui_compositor_update();
    }
  }
}

void ui_compositor_set_frame(char* id, int x, int y, int w, int h)
{
  crect_t* rect;

  //printf("ui_compositor_set_frame %s %i %i %i %i\n", id, x, y, w, h);

  if ((rect = MGET(rectm, id)))
  {
    crect_set_frame(rect, x, y, w, h);
  }

  ui_compositor_update();
}

void ui_compositor_set_texture(char* id, bm_t* tex)
{
  // printf("ui_compositor_set_texture %s\n", id);

  crect_t*    rect;
  tm_coords_t coords;

  if ((rect = MGET(rectm, id)))
  {
    coords = tm_get(tm, id);

    if (coords.w != tex->w || coords.h != tex->h)
    {
      // printf("ui_compositor text2ure size mismatch, adding as new %s %i %i %i %i\n", id, coords.w, tex->w, coords.h, tex->h);
      tm_put(tm, id, tex);
      coords = tm_get(tm, id);
      // printf("new coords %f %f %f %f\n", coords.ltx, coords.lty, coords.rbx, coords.rby);
    }
    else
    {
      // printf("ui_compositor updating texture in place %s\n", id);
      tm_upd(tm, id, tex);
    }

    crect_set_texture(rect, coords.ltx, coords.lty, coords.rbx, coords.rby);
    ui_compositor_update();
  }
}

void ui_compositor_resize(int width, int height)
{
  comp_width  = width;
  comp_height = height;
}

//
// Compositor Rect
//

crect_t* crect_new(char*    id,
                   uint32_t index,
                   uint32_t channel,
                   float    x,
                   float    y,
                   float    w,
                   float    h,
                   float    tx,
                   float    ty,
                   float    tz,
                   float    tw)
{
  crect_t* r = mtmem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  r->id    = mtcstr_fromcstring(id);
  r->index = index;

  r->region = ((region_t){x, y, w, h});

  r->data[0] = x;
  r->data[1] = y;
  r->data[2] = tx;
  r->data[3] = ty;
  r->data[4] = (float)channel;

  r->data[5] = x + w;
  r->data[6] = y + h;
  r->data[7] = tz;
  r->data[8] = tw;
  r->data[9] = (float)channel;

  r->data[10] = x;
  r->data[11] = y + h;
  r->data[12] = tx;
  r->data[13] = tw;
  r->data[14] = (float)channel;

  r->data[15] = x + w;
  r->data[16] = y;
  r->data[17] = tz;
  r->data[18] = ty;
  r->data[19] = (float)channel;

  r->data[20] = x;
  r->data[21] = y;
  r->data[22] = tx;
  r->data[23] = ty;
  r->data[24] = (float)channel;

  r->data[25] = x + w;
  r->data[26] = y + h;
  r->data[27] = tz;
  r->data[28] = tw;
  r->data[29] = (float)channel;

  return r;
}

void crect_del(void* pointer)
{
  crect_t* r = (crect_t*)pointer;
  REL(r->id);
}

void crect_set_frame(crect_t* r, float x, float y, float w, float h)
{
  r->region = ((region_t){x, y, w, h});

  r->data[0] = x;
  r->data[1] = y;

  r->data[5] = x + w;
  r->data[6] = y + h;

  r->data[10] = x;
  r->data[11] = y + h;

  r->data[15] = x + w;
  r->data[16] = y;

  r->data[20] = x;
  r->data[21] = y;

  r->data[25] = x + w;
  r->data[26] = y + h;
}

void crect_set_texture(crect_t* r, float tx, float ty, float tz, float tw)
{
  r->data[2] = tx;
  r->data[3] = ty;

  r->data[7] = tz;
  r->data[8] = tw;

  r->data[12] = tx;
  r->data[13] = tw;

  r->data[17] = tz;
  r->data[18] = ty;

  r->data[22] = tx;
  r->data[23] = ty;

  r->data[27] = tz;
  r->data[28] = tw;
}

void crect_desc(crect_t* r)
{
  for (int index = 0; index < 30; index++)
  {
    if (index % 5 == 0) printf("\n");
    printf("%f ", r->data[index]);
  }
  printf("\n");
}

#endif
