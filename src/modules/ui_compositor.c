/*
  UI Compositor Module for Zen Multimedia Desktop System
  Places incoming bitmaps in texture maps, incoming rectangles and texture map positions in floatbuffers, renders rects on demand.

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "gl_connector.c"
#include "mtbitmap.c"

typedef struct _uirect_t
{
  float x;
  float y;
  float w;
  float h;
} uirect_t;

typedef struct _uitexc_t
{
  float x;
  float y;
  float z;
  float w;
} uitexc_t;

void ui_compositor_init(int, int);
void ui_compositor_render();
void ui_compositor_reset();
void ui_compositor_add(char* id, uint32_t index, uirect_t uirect, int page, char shadow, char blurred);
void ui_compositor_rem(char* id);
void ui_compositor_set_index(char* id, uint32_t index);
void ui_compositor_set_frame(char* id, uirect_t rect);
void ui_compositor_set_texture(char* viewid, char* texid, bm_t* bmp);
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

crect_t* crect_new(char* id, uint32_t index, uint32_t channel, uirect_t rect, uitexc_t texc);
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_index(crect_t* rest, uint32_t index);
void     crect_set_frame(crect_t* rect, uirect_t uirect);
void     crect_set_texture(crect_t* rect, uitexc_t texc);

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

  gl_get_texture(1, 4096, 4096); /* init texture for ui texture map */

  gl_get_texture(3, 4096, 4096); /* init texture for ui texture map */
  gl_get_texture(4, 4096, 4096); /* init texture for ui texture map */
  gl_get_texture(5, 4096, 4096); /* init texture for ui texture map */
  gl_get_texture(6, 4096, 4096); /* init texture for ui texture map */
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
  gl_update_textures(1, tm->bm);
  gl_clear_framebuffer(0, 0.01, 0.01, 0.01, 1.0);
  gl_clear_framebuffer(3, 0.01, 0.01, 0.01, 1.0);

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
    //printf("adding vertexs for %s index %i\n", rect->id, rect->index);
    //crect_desc(rect);
    fb_add(fb, rect->data, 30);
  }
}

void ui_compositor_add(char* id, uint32_t index, uirect_t uirect, int page, char shadow, char blur)
{
  printf("ui_compositor_add %s index %i page %i %f %f %f %f\n", id, index, page, uirect.x, uirect.y, uirect.w, uirect.h);

  gltex_t  tex_dim = gl_get_texture(page, uirect.w, uirect.h);
  uitexc_t tex_cor = (uitexc_t){0.0, 0.0, uirect.w / tex_dim.w, uirect.h / tex_dim.h};

  crect_t* rect = crect_new(id, index, page, uirect, tex_cor);

  rect->shadow = shadow;
  rect->blur   = blur;

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

void ui_compositor_set_frame(char* id, uirect_t uirect)
{
  // printf("ui_compositor_set_frame %s %f %f %f %f\n", id, uirect.x, uirect.y, uirect.w, uirect.h);

  crect_t* rect;

  if ((rect = MGET(rectm, id)))
  {
    crect_set_frame(rect, uirect);
  }

  ui_compositor_update();
}

void ui_compositor_set_texture(char* viewid, char* texid, bm_t* tex)
{
  crect_t*    rect;
  tm_coords_t coords;

  if ((rect = MGET(rectm, viewid)))
  {
    coords = tm_get(tm, texid);

    if (coords.w != tex->w || coords.h != tex->h)
    {
      // printf("ui_compositor text2ure size mismatch, adding as new %s %i %i %i %i\n", id, coords.w, tex->w, coords.h, tex->h);
      tm_put(tm, texid, tex);
      coords = tm_get(tm, texid);
    }
    else
    {
      // printf("ui_compositor updating texture in place %s\n", id);
      tm_upd(tm, texid, tex);
    }

    crect_set_texture(rect, (uitexc_t){.x = coords.ltx, .y = coords.lty, .z = coords.rbx, .w = coords.rby});
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

crect_t* crect_new(char* id, uint32_t index, uint32_t page, uirect_t rect, uitexc_t texc)
{
  crect_t* r = mtmem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  r->id    = mtcstr_fromcstring(id);
  r->index = index;

  r->region = ((region_t){rect.x, rect.y, rect.w, rect.h});

  r->data[0] = rect.x;
  r->data[1] = rect.y;
  r->data[2] = texc.x;
  r->data[3] = texc.y;
  r->data[4] = (float)page;

  r->data[5] = rect.x + rect.w;
  r->data[6] = rect.y + rect.h;
  r->data[7] = texc.z;
  r->data[8] = texc.w;
  r->data[9] = (float)page;

  r->data[10] = rect.x;
  r->data[11] = rect.y + rect.h;
  r->data[12] = texc.x;
  r->data[13] = texc.w;
  r->data[14] = (float)page;

  r->data[15] = rect.x + rect.w;
  r->data[16] = rect.y;
  r->data[17] = texc.z;
  r->data[18] = texc.y;
  r->data[19] = (float)page;

  r->data[20] = rect.x;
  r->data[21] = rect.y;
  r->data[22] = texc.x;
  r->data[23] = texc.y;
  r->data[24] = (float)page;

  r->data[25] = rect.x + rect.w;
  r->data[26] = rect.y + rect.h;
  r->data[27] = texc.z;
  r->data[28] = texc.w;
  r->data[29] = (float)page;

  return r;
}

void crect_del(void* pointer)
{
  crect_t* r = (crect_t*)pointer;
  REL(r->id);
}

void crect_set_frame(crect_t* r, uirect_t rect)
{
  r->region = ((region_t){rect.x, rect.y, rect.w, rect.h});

  r->data[0] = rect.x;
  r->data[1] = rect.y;

  r->data[5] = rect.x + rect.w;
  r->data[6] = rect.y + rect.h;

  r->data[10] = rect.x;
  r->data[11] = rect.y + rect.h;

  r->data[15] = rect.x + rect.w;
  r->data[16] = rect.y;

  r->data[20] = rect.x;
  r->data[21] = rect.y;

  r->data[25] = rect.x + rect.w;
  r->data[26] = rect.y + rect.h;
}

void crect_set_texture(crect_t* r, uitexc_t texc)
{
  r->data[2] = texc.x;
  r->data[3] = texc.y;

  r->data[7] = texc.z;
  r->data[8] = texc.w;

  r->data[12] = texc.x;
  r->data[13] = texc.w;

  r->data[17] = texc.z;
  r->data[18] = texc.y;

  r->data[22] = texc.x;
  r->data[23] = texc.y;

  r->data[27] = texc.z;
  r->data[28] = texc.w;
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
