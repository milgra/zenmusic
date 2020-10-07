
#ifndef fb_h
#define fb_h

#include "mtmem.c"
#include <GL/glew.h>
#include <string.h>

typedef struct fb_t fb_t;
struct fb_t
{
  GLfloat* data;
  uint32_t pos;
  uint32_t cap;
  char changed;
};

fb_t* fb_alloc(void);
void fb_dealloc(void* fb);
void fb_reset(fb_t* fb);
void fb_app(fb_t* fb, GLfloat data);
void fb_app_arr(fb_t* fb, GLfloat* data, size_t count);

#endif

#if __INCLUDE_LEVEL__ == 0

fb_t* fb_alloc()
{
  fb_t* fb = mtmem_calloc(sizeof(fb_t), fb_dealloc);
  fb->data = mtmem_calloc(sizeof(GLfloat) * 10, NULL);
  fb->pos = 0;
  fb->cap = 10;

  return fb;
}

void fb_dealloc(void* pointer)
{
  fb_t* fb = pointer;
  mtmem_release(fb->data);
}

void fb_reset(fb_t* fb)
{
  fb->pos = 0;
}

void fb_expand(fb_t* fb)
{
  assert(fb->cap < UINT32_MAX / 2);
  fb->cap *= 2;
  fb->data = mtmem_realloc(fb->data, sizeof(void*) * fb->cap);
}

void fb_app(fb_t* fb, GLfloat value)
{
  if (fb->pos == fb->cap) fb_expand(fb);
  fb->data[fb->pos] = value;
  fb->pos += 1;
  fb->changed = 1;
}

void fb_app_arr(fb_t* fb, GLfloat* data, size_t count)
{
  while (fb->pos + count >= fb->cap)
    fb_expand(fb);
  memcpy(fb->data + fb->pos, data, sizeof(GLfloat) * count);
  fb->pos += count;
  fb->changed = 1;
}

#endif
