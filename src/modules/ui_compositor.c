/*
  UI Compositor Module for Zen Multimedia Desktop System
  Stores incoming rects
  Stores incoming textures
  Links rects with textures
  Uploads and renders rects on demand

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "gl_connector.c"
#include "mtbitmap.c"
#include "mtmath2.c"

void ui_compositor_init(int, int);

void ui_compositor_new_reset(r2_t frame, int page, bm_t* tex);
void ui_compositor_new_upd(r2_t  frame,
                           float border, // view border
                           int   page,   // texture page
                           int   full,   // needs full texture
                           int   ext,    // external texture
                           char* texid,  // texture id
                           bm_t* bm);    // texture bitmap

void ui_compositor_new_render();

void ui_compositor_resize(int width, int height);
void ui_compositor_render();

void ui_compositor_reset();

int ui_compositor_map_texture();
int ui_compositor_new_texture();

void ui_compositor_add(char*    rectid,
                       char*    texid,
                       uint32_t index);
void ui_compositor_rem(char* rectid);
void ui_compositor_upd_texture(char* id, int page, int full, int ext, int blur, int w, int h);
void ui_compositor_upd_bitmap(char* texid, bm_t* tex);
void ui_compositor_upd_frame(char* rectid, r2_t frame, float border);
void ui_compositor_upd_index(char* rectid, int index);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_floatbuffer.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtmath4.c"
#include "mttexmap.c"
#include "mtvector.c"

typedef struct _crect_t
{
  char*    id;
  char*    tex_id;
  float    data[30];
  uint32_t index;
  char     ready;
  int      page;
  char     blur;
  char     shadow;
  glrect_t region;
} crect_t;

crect_t* crect_new(char* id, char* tex_id, uint32_t index);
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_frame(crect_t* rect, r2_t uirect);
void     crect_set_page(crect_t* rect, uint32_t page);
void     crect_set_blur(crect_t* rect, char blur);
void     crect_set_shadow(crect_t* rect, char shadow);
void     crect_set_texture(crect_t* rect, float tlx, float tly, float brx, float bry);

struct uic_t
{
  fb_t*  fb;      // float buffer
  tm_t*  tm;      // texture map
  vec_t* rects_v; // rectangle vector
  vec_t* final_v; // rectangle vector for quick sort
  map_t* rects_m; // rectangle map
  int    width;
  int    height;
  int    tex_page;
  int    upd_geo; // update geometry

  vec_t* cache;
  int    cache_ind;
} uic = {0};

void ui_compositor_init(int width, int height)
{
  gl_init();

  uic.width  = width;
  uic.height = height;

  uic.fb      = fb_new();
  uic.tm      = tm_new(4096, 4096);
  uic.rects_v = VNEW();
  uic.final_v = VNEW();
  uic.rects_m = MNEW();

  uic.cache     = VNEW();
  uic.cache_ind = 0;

  /* init texture for ui texture map */

  gl_get_texture(uic.tex_page++, 4096, 4096);

  /* textures for framebuffer composition */

  gl_get_texture(uic.tex_page++, 4096, 4096);
  gl_get_texture(uic.tex_page++, 4096, 4096);
  gl_get_texture(uic.tex_page++, 4096, 4096);
}

void ui_compositor_new_reset(r2_t frame, int page, bm_t* tex)
{
  uic.cache_ind = 0;
}

void ui_compositor_new_upd(r2_t  frame,
                           float border, // view border
                           int   page,   // texture page
                           int   full,   // needs full texture
                           int   ext,    // external texture
                           char* texid,  // texture id
                           bm_t* bm)     // texture bitmap
{
  // fill up cache if needed
  if (uic.cache_ind + 1 > uic.cache->length)
  {
    crect_t* rect = crect_new("", "", 0);
    VADD(uic.cache, rect);
  }
  // get cached rect
  crect_t* rect = uic.cache->data[uic.cache_ind];

  // set frame
  if (border > 0.0) frame = r2_expand(frame, border);
  crect_set_frame(rect, frame);

  // set page
  crect_set_page(rect, page);

  // set texture coords
  if (full)
  {
    // view wants full texture to show
    crect_set_texture(rect, 0.0, 0.0, 1.0, 1.0);
  }

  if (ext && frame.w > 0 && frame.h > 0)
  {
    // use view dimensions as texture dimensions in case of external texture
    glrect_t tex_dim = gl_get_texture(page, frame.w, frame.h);
    crect_set_texture(rect, 0.0, 0.0, frame.w / (float)tex_dim.w, frame.h / (float)tex_dim.h);
  }

  // store bitmap
  tm_coords_t tc = tm_get(uic.tm, texid);

  if (bm->w != tc.w || bm->h != tc.h)
  {
    // texture doesn't exist or size mismatch
    int success = tm_put(uic.tm, texid, bm->w, bm->h);
    // TODO reset main texture, maybe all views?
    if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

    // update tex coords
    tc = tm_get(uic.tm, texid);

    crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);
  }

  // upload to GPU
  gl_upload_to_texture(0, tc.x, tc.y, bm->w, bm->h, bm->data);

  // increase cache index
  uic.cache_ind++;
}

void ui_compositor_new_render()
{
  fb_reset(uic.fb);

  for (int i = 0; i < uic.cache_ind; i++)
  {
    crect_t* rect = uic.cache->data[i];
    fb_add(uic.fb, rect->data, 30);
  }

  gl_upload_vertexes(uic.fb);
  gl_clear_framebuffer(TEX_CTX, 0.01, 0.01, 0.01, 1.0);

  glrect_t reg_full = {0, 0, uic.width, uic.height};

  gl_draw_vertexes_in_framebuffer(TEX_CTX, 0, uic.fb->pos / 5, reg_full, reg_full, SH_TEXTURE);
}

void ui_compositor_reset()
{
  fb_reset(uic.fb);
  tm_reset(uic.tm);

  vec_reset(uic.rects_v);
  vec_reset(uic.final_v);
  map_reset(uic.rects_m);
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

void ui_compositor_add(char* rectid, char* texid, uint32_t index)
{
  // printf("ui_compositor_add rectid %s texid %s index %i\n", rectid, texid, index);

  crect_t* rect = map_get(uic.rects_m, rectid);

  if (rect == NULL)
  {
    crect_t* rect = crect_new(rectid, texid, index);

    VADD(uic.rects_v, rect);
    VADD(uic.final_v, rect);
    MPUT(uic.rects_m, rectid, rect);

    REL(rect);

    uic.upd_geo = 1;
  }
  else
    printf("EXISTING CRECT IN UI COMPOSITOR : %s\n", rectid);
}

void ui_compositor_rem(char* id)
{
  // printf("REMOVE ui_compositor_rem %s\n", id);
  crect_t* rect = map_get(uic.rects_m, id);

  if (rect)
  {
    VREM(uic.rects_v, rect);
    VREM(uic.final_v, rect);
    MDEL(uic.rects_m, id);

    uic.upd_geo = 1;
  }
}

void ui_compositor_upd_texture(char* id, int page, int full, int ext, int blur, int w, int h)
{
  // printf("ui_compositor_upd_texture %s %i\n", id, page);
  crect_t* rect = map_get(uic.rects_m, id);

  if (rect)
  {
    crect_set_blur(rect, blur);

    // TODO set this in individual function
    if (full)
    {
      // view wants full texture to show, no tex update will come so we set tex coords here
      crect_set_texture(rect, 0.0, 0.0, 1.0, 1.0);
    }

    if (ext && w > 0 && h > 0)
    {
      // use view dimensions as texture dimensions in case of external texture
      glrect_t tex_dim = gl_get_texture(page, w, h);
      crect_set_texture(rect, 0.0, 0.0, w / (float)tex_dim.w, h / (float)tex_dim.h);
    }

    crect_set_page(rect, page);

    uic.upd_geo = 1;
  }
}

void ui_compositor_upd_frame(char* rectid, r2_t frame, float border)
{
  crect_t* rect = map_get(uic.rects_m, rectid);

  if (rect)
  {
    if (border > 0.0) frame = r2_expand(frame, border);
    crect_set_frame(rect, frame);
    uic.upd_geo = 1;
  }
}

void ui_compositor_upd_bitmap(char* texid, bm_t* bm)
{
  // printf("update bitmap %s %i %i\n", texid, bm->w, bm->h);

  tm_coords_t tc = tm_get(uic.tm, texid);

  if (bm->w != tc.w || bm->h != tc.h)
  {
    // texture doesn't exist or size mismatch
    int success = tm_put(uic.tm, texid, bm->w, bm->h);
    // TODO reset main texture, maybe all views?
    if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

    // update tex coords
    tc = tm_get(uic.tm, texid);

    crect_t* rect;

    // go through all rects, update texture coord if texid is identical
    while ((rect = VNXT(uic.rects_v)))
    {
      if (strcmp(rect->tex_id, texid) == 0)
      {
        crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);
        uic.upd_geo = 1;
      }
    }
  }

  // upload to GPU
  gl_upload_to_texture(0, tc.x, tc.y, bm->w, bm->h, bm->data);
}

void ui_compositor_upd_index(char* rectid, int index)
{
  crect_t* rect = map_get(uic.rects_m, rectid);

  if (rect)
  {
    rect->index = index;
    uic.upd_geo = 1;
  }
}

void ui_compositor_render()
{
  if (uic.upd_geo)
  {
    fb_reset(uic.fb);

    crect_t* rect;

    // set sequence
    while ((rect = VNXT(uic.rects_v)))
    {
      if (rect->index < uic.final_v->length) vec_replaceatindex(uic.final_v, rect, rect->index);
    }

    while ((rect = VNXT(uic.final_v)))
    {
      if (rect->ready) fb_add(uic.fb, rect->data, 30);
      //printf("adding vertexs for %s index %i\n", rect->id, rect->index);
      //crect_desc(rect);
    }

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

crect_t* crect_new(char* id, char* texid, uint32_t index)
{
  crect_t* r = mem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  r->id     = cstr_fromcstring(id);
  r->tex_id = cstr_fromcstring(texid);
  r->index  = index;

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

void crect_set_texture(crect_t* r, float tlx, float tly, float brx, float bry)
{
  r->ready = 1;

  r->data[2] = tlx;
  r->data[3] = tly;

  r->data[7] = brx;
  r->data[8] = bry;

  r->data[12] = tlx;
  r->data[13] = bry;

  r->data[17] = brx;
  r->data[18] = tly;

  r->data[22] = tlx;
  r->data[23] = tly;

  r->data[27] = brx;
  r->data[28] = bry;
}

void crect_set_page(crect_t* r, uint32_t page)
{
  r->page     = page;
  r->data[4]  = (float)page;
  r->data[9]  = (float)page;
  r->data[14] = (float)page;
  r->data[19] = (float)page;
  r->data[24] = (float)page;
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
