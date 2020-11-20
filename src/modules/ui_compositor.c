/*
  UI Compositor Module for Zen Multimedia Desktop System
  Places incoming bitmaps in texture maps, incoming rectangles and texture map positions in floatbuffers, renders rects on demand.

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "gl_connector.c"
#include "mtbitmap.c"
#include "mtmath2.c"

void ui_compositor_init(int, int);
void ui_compositor_reset();
void ui_compositor_resize(int width, int height);
void ui_compositor_render();
void ui_compositor_add(char* viewid, r2_t uirect, uint32_t index);
void ui_compositor_rem(char* id);
void ui_compositor_set_index(char* id, uint32_t index);
void ui_compositor_set_frame(char* id, r2_t rect);
int  ui_compositor_map_texture();
int  ui_compositor_new_texture();
void ui_compositor_set_texture(char* viewid, char* texid, bm_t* tex, int page, char shadow, char blur, char full);
int  ui_compositor_has_texture(char* texid);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_floatbuffer.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtmath4.c"
#include "mttexmap.c"
#include "mtvector.c"

typedef struct _uitexc_t
{
  float x;
  float y;
  float z;
  float w;
} uitexc_t;

typedef struct _crect_t
{
  char*    id;
  float    data[30];
  uint32_t index;
  char     ready;
  char     blur;
  char     shadow;
  glrect_t region;
} crect_t;

crect_t* crect_new(char* id, r2_t rect, uint32_t index);
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_blur(crect_t* rect, char blur);
void     crect_set_shadow(crect_t* rect, char shadow);
void     crect_set_frame(crect_t* rect, r2_t uirect);
void     crect_set_texture(crect_t* rect, uitexc_t texc, uint32_t page);

struct uic_t
{
  fb_t*    fb;      // float buffer
  tm_t*    tm;      // texture map
  mtvec_t* rects_v; // rectangle vector
  mtvec_t* final_v; // rectangle vector
  mtmap_t* rects_m; // rectangle map
  int      width;
  int      height;
  int      tex_page;
  int      upd_geo; // update geometry
} uic = {0};

void ui_compositor_init(int width, int height)
{
  gl_init(width, height);

  uic.fb      = fb_new();
  uic.tm      = tm_new(4096, 4096);
  uic.rects_v = VNEW();
  uic.final_v = VNEW();
  uic.rects_m = MNEW();

  /* init texture for ui texture map */

  gl_get_texture(uic.tex_page++, 4096, 4096);

  /* textures for framebuffer composition */

  gl_get_texture(uic.tex_page++, 4096, 4096);
  gl_get_texture(uic.tex_page++, 4096, 4096);
  gl_get_texture(uic.tex_page++, 4096, 4096);
}

void ui_compositor_reset()
{
  fb_reset(uic.fb);
  tm_reset(uic.tm);
  mtvec_reset(uic.rects_v);
  mtmap_reset(uic.rects_m);
}

void ui_compositor_resize(int width, int height)
{
  uic.width  = width;
  uic.height = height;
}

int ui_compositor_new_texture()
{
  return uic.tex_page++;
}

int ui_compositor_map_texture()
{
  return 0;
}

void ui_compositor_add(char* viewid, r2_t uirect, uint32_t index)
{
  // printf("ui_compositor_add viewid %s rect %f %f %f %f index %i\n", viewid, uirect.x, uirect.y, uirect.w, uirect.h, index);

  crect_t* rect = crect_new(viewid, uirect, index);

  VADD(uic.rects_v, rect);
  VADD(uic.final_v, rect);
  MPUT(uic.rects_m, viewid, rect);

  uic.upd_geo = 1;
}

void ui_compositor_rem(char* id)
{
  // printf("ui_compositor_rem %s\n", id);
  crect_t* rect;

  rect = mtmap_get(uic.rects_m, id);
  MDEL(uic.rects_m, id);
  VREM(uic.rects_v, rect);
  VREM(uic.final_v, rect);

  uic.upd_geo = 1;
}

void ui_compositor_set_index(char* id, uint32_t index)
{
  // printf("ui_compositor_set_index %s %i\n", id, index);

  crect_t* rect;

  if ((rect = MGET(uic.rects_m, id)))
  {
    rect->index = index;
    uic.upd_geo = 1;
  }
}

void ui_compositor_set_frame(char* id, r2_t uirect)
{
  // printf("ui_compositor_set_frame %s %f %f %f %f\n", id, uirect.x, uirect.y, uirect.w, uirect.h);

  crect_t* rect;

  if ((rect = MGET(uic.rects_m, id)))
  {
    crect_set_frame(rect, uirect);
    uic.upd_geo = 1;
  }
}

void ui_compositor_set_texture(char* viewid, char* texid, bm_t* tex, int page, char shadow, char blur, char full)
{
  //printf("ui_compositor_set_texture %s %s\n", viewid, texid);
  crect_t* rect;

  if ((rect = MGET(uic.rects_m, viewid)))
  {
    tm_coords_t coords  = tm_get(uic.tm, texid);
    uitexc_t    tex_cor = {0};

    crect_set_blur(rect, blur);
    crect_set_shadow(rect, shadow);

    if (coords.w > 0)
    {
      // texture exists for view, refresh texture
      tex_cor = (uitexc_t){coords.ltx, coords.lty, coords.rbx, coords.rby};
      gl_upload_to_texture(0, coords.x, coords.y, tex->w, tex->h, tex->data);
      crect_set_texture(rect, tex_cor, page);
    }
    else if (full)
    {
      // view wants full texture to show
      tex_cor = (uitexc_t){0.0, 0.0, 1.0, 1.0};
      crect_set_texture(rect, tex_cor, page);
      uic.upd_geo = 1;
    }
    else if (page > 0)
    {
      // external textures use view size part of the texture
      glrect_t tex_dim = gl_get_texture(page, rect->region.w, rect->region.h);
      uitexc_t tex_cor = (uitexc_t){0.0, 0.0, rect->region.w / (float)tex_dim.w, rect->region.h / (float)tex_dim.h};
      crect_set_texture(rect, tex_cor, page);
      uic.upd_geo = 1;
    }
    else
    {
      // texture doesn't exist
      int success = tm_put(uic.tm, texid, tex);
      // TODO reset main texture, maybe all views?
      if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

      coords = tm_get(uic.tm, texid);
      gl_upload_to_texture(0, coords.x, coords.y, tex->w, tex->h, tex->data);

      uitexc_t tex_cor = (uitexc_t){coords.ltx, coords.lty, coords.rbx, coords.rby};
      crect_set_texture(rect, tex_cor, page);
      uic.upd_geo = 1;
    }
  }
}

int ui_compositor_has_texture(char* texid)
{
  tm_coords_t coord = tm_get(uic.tm, texid);
  if (coord.w > 0 && coord.h > 0)
    return 1;
  else
    return 0;
}

void ui_compositor_render()
{
  if (uic.upd_geo)
  {
    fb_reset(uic.fb);

    crect_t* rect;

    // set sequence
    while ((rect = VNXT(uic.rects_v)))
      mtvec_replaceatindex(uic.final_v, rect, rect->index);

    while ((rect = VNXT(uic.final_v)))
      if (rect->ready) fb_add(uic.fb, rect->data, 30);
    //printf("adding vertexs for %s index %i\n", rect->id, rect->index);
    //crect_desc(rect);

    uic.upd_geo = 0;
    gl_upload_vertexes(uic.fb);
  }

  gl_clear_framebuffer(TEX_CTX, 0.01, 0.01, 0.01, 1.0);

  glrect_t reg_full = {0, 0, uic.width, uic.height};
  glrect_t reg_half = {0, 0, uic.width / 2, uic.height / 2};

  gl_draw_vertexes_in_framebuffer(TEX_CTX, 0, uic.fb->pos / 5, reg_full, reg_full, SH_TEXTURE);
  return;

  crect_t* rect;
  int      last  = 0;
  int      index = 0;
  for (index = 0; index < uic.final_v->length; index++)
  {
    rect = uic.final_v->data[index];
    if (rect->shadow || rect->blur)
    {
      // render rects so far with simple texture renderer to offscreen buffer
      gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_TEXTURE);

      last = index;

      if (rect->shadow)
      {
        // render current view with black color to an offscreen buffer
        gl_clear_framebuffer(2, 0.0, 0.0, 0.0, 0.0);
        gl_clear_framebuffer(3, 0.0, 0.0, 0.0, 0.0);
        gl_draw_vertexes_in_framebuffer(2, index * 6, (index + 1) * 6, reg_full, reg_half, SH_COLOR);
        // blur offscreen buffer for soft shadows
        gl_draw_framebuffer_in_framebuffer(2, 3, reg_half, reg_half, ((glrect_t){0}), SH_BLUR);
        // draw offscreen buffer on final buffer
        gl_draw_framebuffer_in_framebuffer(3, 1, reg_half, reg_full, ((glrect_t){0}), SH_DRAW);
      }

      if (rect->blur)
      {
        // render current state with texture shader to an offscreen buffer
        gl_clear_framebuffer(6, 0.0, 0.0, 0.0, 0.0);
        gl_clear_framebuffer(5, 0.0, 0.0, 0.0, 0.0);
        // shrink current framebuffer for blur
        gl_draw_framebuffer_in_framebuffer(3, 6, reg_full, reg_half, ((glrect_t){0}), SH_DRAW);

        // blur offscreen buffer for soft shadows
        gl_draw_framebuffer_in_framebuffer(6, 5, reg_half, reg_half, ((glrect_t){0}), SH_BLUR);
        gl_draw_framebuffer_in_framebuffer(5, 6, reg_half, reg_half, ((glrect_t){0}), SH_BLUR);

        // draw blurred buffer on final buffer inside the view
        gl_draw_framebuffer_in_framebuffer(6, 3, reg_half, reg_full, rect->region, SH_DRAW);

        // skip drawing actual rect when blur
        last++;
        index++;
      }
    }
  }

  if (last < index)
  {
    // render remaining
    gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_DRAW);
  }

  // finally draw offscreen buffer to screen buffer
  gl_draw_framebuffer_in_framebuffer(1, TEX_CTX, reg_full, reg_full, ((glrect_t){0}), SH_DRAW);
}

//
// Compositor Rect
//

crect_t* crect_new(char* id, r2_t rect, uint32_t index)
{
  crect_t* r = mtmem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  r->id    = mtcstr_fromcstring(id);
  r->index = index;
  r->ready = 0;

  r->region = ((glrect_t){rect.x, rect.y, rect.w, rect.h});

  crect_set_frame(r, rect);

  return r;
}

void crect_del(void* pointer)
{
  crect_t* r = (crect_t*)pointer;
  REL(r->id);
}

void crect_set_ready(crect_t* r, char ready)
{
  r->ready = ready;
}

void crect_set_blur(crect_t* r, char blur)
{
  r->blur = blur;
}

void crect_set_shadow(crect_t* r, char shadow)
{
  r->shadow = shadow;
}

void crect_set_frame(crect_t* r, r2_t rect)
{
  r->region = ((glrect_t){rect.x, rect.y, rect.w, rect.h});

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

void crect_set_texture(crect_t* r, uitexc_t texc, uint32_t page)
{
  r->ready = 1;

  r->data[2] = texc.x;
  r->data[3] = texc.y;
  r->data[4] = (float)page;

  r->data[7] = texc.z;
  r->data[8] = texc.w;
  r->data[9] = (float)page;

  r->data[12] = texc.x;
  r->data[13] = texc.w;
  r->data[14] = (float)page;

  r->data[17] = texc.z;
  r->data[18] = texc.y;
  r->data[19] = (float)page;

  r->data[22] = texc.x;
  r->data[23] = texc.y;
  r->data[24] = (float)page;

  r->data[27] = texc.z;
  r->data[28] = texc.w;
  r->data[29] = (float)page;
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
