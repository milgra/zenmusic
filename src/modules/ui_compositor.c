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
void ui_compositor_reset();
void ui_compositor_resize(int width, int height);

int ui_compositor_new_texture();
int ui_compositor_map_texture();

void ui_compositor_rewind();
void ui_compositor_add(char* id,
                       char  masked,
                       char  hidden,
                       r2_t  frame,
                       float border, // view border
                       int   page,   // texture page
                       int   full,   // needs full texture
                       int   ext,    // external texture
                       char* texid); // texture id
void ui_compositor_upd_pos(int index, r2_t frame, float border);
void ui_compositor_upd_bmp(int index, r2_t frame, float border, char* texid, bm_t* bm);
void ui_compositor_upd_vis(int index, char hidden);
void ui_compositor_render(uint32_t time);

#endif

#if __INCLUDE_LEVEL__ == 0

#define UI_STAT_DELAY 10.0

#include "gl_floatbuffer.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtmath4.c"
#include "mttexmap.c"
#include "mtvector.c"

typedef struct _crect_t
{
  char* id;
  char* tex_id;
  float data[30];
  char  hidden;
  char  masked;
  r2_t  frame;
} crect_t;

crect_t* crect_new();
void     crect_del(void* rect);
void     crect_desc(crect_t* rect);
void     crect_set_id(crect_t* rect, char* id);
void     crect_set_hidden(crect_t* r, char hidden);
void     crect_set_masked(crect_t* r, char masked);
void     crect_set_page(crect_t* rect, uint32_t page);
void     crect_set_frame(crect_t* rect, r2_t uirect);
void     crect_set_texture(crect_t* rect, float tlx, float tly, float brx, float bry);

struct uic_t
{
  fb_t* fb; // float buffer
  tm_t* tm; // texture map
  int   width;
  int   height;
  int   tex_page;
  int   upd_geo; // update geometry

  vec_t* cache;
  int    cache_ind;

  uint32_t tex_bytes;
  uint32_t ver_bytes;
  uint32_t upd_stamp;
  uint32_t ren_frame;
} uic = {0};

void ui_compositor_init(int width, int height)
{
  gl_init();

  uic.width  = width;
  uic.height = height;

  uic.fb = fb_new();
  uic.tm = tm_new(4096, 4096);

  uic.cache     = VNEW();
  uic.cache_ind = 0;
  uic.upd_geo   = 1;

  // TODO texture dimensions should be bigger than screen size
  gl_get_texture(0, 4096, 4096); // texture for texmap
  gl_get_texture(1, 4096, 4096); // texture for mask

  uic.tex_page = 2; // new textures should start from 2
}

void ui_compositor_rewind()
{
  uic.cache_ind = 0;
}

void ui_compositor_reset()
{
  fb_reset(uic.fb);
  tm_reset(uic.tm);

  uic.cache_ind = 0;
  uic.upd_geo   = 1;
}

void ui_compositor_resize(int width, int height)
{
  uic.width   = width;
  uic.height  = height;
  uic.upd_geo = 1;
}

int ui_compositor_new_texture()
{
  return uic.tex_page++;
}

int ui_compositor_map_texture()
{
  return 0;
}

void ui_compositor_add(char* id,
                       char  masked,
                       char  hidden,
                       r2_t  frame,
                       float border, // view border
                       int   page,   // texture page
                       int   full,   // needs full texture
                       int   ext,    // external texture
                       char* texid)  // texture id
{
  // printf("COMP ADD %s %f %f %f %f masked %i\n", id, frame.x, frame.y, frame.w, frame.h, masked);

  // fill up cache if needed
  if (uic.cache_ind + 1 > uic.cache->length)
  {
    crect_t* rect = crect_new();
    VADD(uic.cache, rect);
  }
  // get cached rect
  crect_t* rect = uic.cache->data[uic.cache_ind];

  // set id
  crect_set_id(rect, id);

  // set hidden
  crect_set_hidden(rect, hidden);

  // set masked
  crect_set_masked(rect, masked);

  // set frame
  if (border > 0.0) frame = r2_expand(frame, border);
  crect_set_frame(rect, frame);

  // set page
  crect_set_page(rect, page);

  // TEXTURE COORDS

  // by default view gets full texture
  crect_set_texture(rect, 0.0, 0.0, 0.0, 0.0);

  if (full)
  {
    crect_set_texture(rect, 0.0, 0.0, 1.0, 1.0);
  }
  else if (ext)
  {
    if (frame.w > 0 && frame.h > 0)
    {
      // use view dimensions as texture dimensions in case of external texture
      glrect_t tex_dim = gl_get_texture(page, frame.w, frame.h);
      crect_set_texture(rect, 0.0, 0.0, frame.w / (float)tex_dim.w, frame.h / (float)tex_dim.h);
    }
  }
  else
  {
    // prepare texmap
    tm_coords_t tc = tm_get(uic.tm, texid);

    if (tc.w > 0 && tc.h > 0)
    {

      if ((int)frame.w != tc.w || (int)frame.h != tc.h)
      {
        // texture doesn't exist or size mismatch
        int success = tm_put(uic.tm, texid, frame.w, frame.h);
        // TODO reset main texture, maybe all views?
        if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

        // update tex coords
        tc = tm_get(uic.tm, texid);
      }

      // set texture coords
      crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);
    }
  }

  // increase cache index
  uic.cache_ind++;
}

void ui_compositor_upd_vis(int index, char hidden)
{
  crect_t* rect = uic.cache->data[index];
  rect->hidden  = hidden;
}

void ui_compositor_upd_pos(int index, r2_t frame, float border)
{
  crect_t* rect  = uic.cache->data[index];
  r2_t     prevf = rect->frame;
  prevf.x        = frame.x - border;
  prevf.y        = frame.y - border;

  crect_set_frame(rect, prevf);

  uic.upd_geo = 1;
}

void ui_compositor_upd_bmp(int index, r2_t frame, float border, char* texid, bm_t* bm)
{
  crect_t* rect = uic.cache->data[index];

  frame.w = bm->w - 2 * border;
  frame.h = bm->h - 2 * border;

  if (border > 0.0) frame = r2_expand(frame, border);
  crect_set_frame(rect, frame);

  tm_coords_t tc = tm_get(uic.tm, texid);

  if (bm->w != tc.w || bm->h != tc.h)
  {
    // texture doesn't exist or size mismatch
    int success = tm_put(uic.tm, texid, frame.w, frame.h);
    // TODO reset main texture, maybe all views?
    if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

    // update tex coords
    tc = tm_get(uic.tm, texid);

    // set new texture coords
    crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);
  }

  // upload to GPU
  gl_upload_to_texture(0, tc.x, tc.y, bm->w, bm->h, bm->data);

  uic.tex_bytes += bm->size;

  uic.upd_geo = 1;
}

void ui_compositor_render(uint32_t time)
{
  if (uic.upd_geo == 1)
  {
    fb_reset(uic.fb);
    for (int i = 0; i < uic.cache_ind; i++)
    {
      crect_t* rect = uic.cache->data[i];
      if (!rect->hidden) fb_add(uic.fb, rect->data, 30);
    }

    gl_upload_vertexes(uic.fb);

    uic.upd_geo = 0;
    uic.ver_bytes += uic.fb->pos * sizeof(GLfloat);
  }

  uic.ren_frame += 1;

  /* int last  = 0; */
  /* int index = 0; */

  /* for (int i = 0; i < uic.cache_ind; i++) */
  /* { */
  /*   crect_t* rect = uic.cache->data[i]; */
  /*   if (!rect->hidden) fb_add(uic.fb, rect->data, 30); */
  /* } */

  // if view is mask, render to mask
  // glrect_t reg_full = {0, 0, uic.width, uic.height};
  // gl_draw_vertexes_in_framebuffer(8, 0, uic.fb->pos / 5, reg_full, reg_full, SH_TEXTURE);

  gl_clear_framebuffer(1, 0, 0, 0, 1.0);
  gl_clear_framebuffer(TEX_CTX, 0.8, 0.8, 0.8, 1.0);
  glrect_t reg_full = {0, 0, uic.width, uic.height};
  gl_draw_vertexes_in_framebuffer(TEX_CTX, 0, uic.fb->pos / 5, reg_full, reg_full, SH_TEXTURE);

  if (time > uic.upd_stamp)
  {
    printf("UI TX %.2f Mb/s VX %.2f Mb/s FPS %.2f\n",
           uic.tex_bytes / UI_STAT_DELAY / (1024.0 * 1024.0),
           uic.ver_bytes / UI_STAT_DELAY / (1024.0 * 1024.0),
           uic.ren_frame / UI_STAT_DELAY);
    uic.upd_stamp = time + UI_STAT_DELAY * 1000.0;
    uic.tex_bytes = 0;
    uic.ver_bytes = 0;
    uic.ren_frame = 0;
  }
}

/* void ui_compositor_render() */
/* { */

/*   crect_t* rect; */
/*   int      last  = 0; */
/*   int      index = 0; */
/*   for (index = 0; index < uic.final_v->length; index++) */
/*   { */
/*     rect = uic.final_v->data[index]; */
/*     if (rect->shadow || rect->blur) */
/*     { */
/*       // render rects so far with simple texture renderer to offscreen buffer */
/*       gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_TEXTURE); */

/*       last = index; */

/*       if (rect->shadow) */
/*       { */
/*         // render current view with black color to an offscreen buffer */
/*         gl_clear_framebuffer(2, 0.0, 0.0, 0.0, 0.0); */
/*         gl_clear_framebuffer(3, 0.0, 0.0, 0.0, 0.0); */
/*         gl_draw_vertexes_in_framebuffer(2, index * 6, (index + 1) * 6, reg_full, reg_half, SH_COLOR); */
/*         // blur offscreen buffer for soft shadows */
/*         gl_draw_framebuffer_in_framebuffer(2, 3, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */
/*         // draw offscreen buffer on final buffer */
/*         gl_draw_framebuffer_in_framebuffer(3, 1, reg_half, reg_full, ((glrect_t){0}), SH_DRAW); */
/*       } */

/*       if (rect->blur) */
/*       { */
/*         // render current state with texture shader to an offscreen buffer */
/*         gl_clear_framebuffer(6, 0.0, 0.0, 0.0, 0.0); */
/*         gl_clear_framebuffer(5, 0.0, 0.0, 0.0, 0.0); */
/*         // shrink current framebuffer for blur */
/*         gl_draw_framebuffer_in_framebuffer(3, 6, reg_full, reg_half, ((glrect_t){0}), SH_DRAW); */

/*         // blur offscreen buffer for soft shadows */
/*         gl_draw_framebuffer_in_framebuffer(6, 5, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */
/*         gl_draw_framebuffer_in_framebuffer(5, 6, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */

/*         // draw blurred buffer on final buffer inside the view */
/*         gl_draw_framebuffer_in_framebuffer(6, 3, reg_half, reg_full, rect->region, SH_DRAW); */

/*         // skip drawing actual rect when blur */
/*         last++; */
/*         index++; */
/*       } */
/*     } */
/*   } */

/*   if (last < index) */
/*   { */
/*     // render remaining */
/*     gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_DRAW); */
/*   } */

/*   // finally draw offscreen buffer to screen buffer */
/*   gl_draw_framebuffer_in_framebuffer(1, TEX_CTX, reg_full, reg_full, ((glrect_t){0}), SH_DRAW); */
/* } */

//
// Compositor Rect
//

crect_t* crect_new(char* id, char* texid, uint32_t index)
{
  crect_t* r = mem_calloc(sizeof(crect_t), "crect_t", crect_del, NULL);

  return r;
}

void crect_del(void* pointer)
{
  crect_t* r = (crect_t*)pointer;
  REL(r->id);
}

void crect_set_id(crect_t* r, char* id)
{
  r->id = id;
}

void crect_set_hidden(crect_t* r, char hidden)
{
  r->hidden = hidden;
}

void crect_set_masked(crect_t* r, char masked)
{
  r->masked = masked;
}

void crect_set_frame(crect_t* r, r2_t rect)
{
  r->frame = rect;

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
