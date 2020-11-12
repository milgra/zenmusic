/*
  OpenGL Connector Module for Zen Multimedia Desktop System
  Renders textured triangles to framebuffers and combines framebuffers with different shaders
  Textures can be internal or external
 */

#ifndef gl_connector_h
#define gl_connector_h

#include "gl_floatbuffer.c"
#include "gl_shader.c"
#include "mtbitmap.c"
#include "mtmath4.c"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum _gl_sha_typ_t
{
  SH_TEXTURE,
  SH_COLOR,
  SH_BLUR
} gl_sha_typ_t;

typedef struct _glrect_t
{
  int x;
  int y;
  int w;
  int h;
} glrect_t;

void     gl_init();
glrect_t gl_get_texture(uint32_t i, uint32_t w, uint32_t h);
void     gl_update_vertexes(fb_t* fb);
void     gl_update_textures(int page, bm_t* bmp);
void     gl_clear_framebuffer(int page, float r, float g, float b, float a);
void     gl_draw_vertexes_in_framebuffer(int page, int start, int end, glrect_t source_region, glrect_t target_region, gl_sha_typ_t shader);
void     gl_draw_framebuffer_in_framebuffer(int src_ind, int tgt_ind, glrect_t source_region, glrect_t target_region, glrect_t window, gl_sha_typ_t shader);
void     gl_draw_to_texture(int page, int w, int h, void* data);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _glbuf_t
{
  GLuint vbo;
  GLuint vao;
  fb_t*  flo_buf;
} glbuf_t;

typedef struct _gltex_t
{
  GLuint index;
  GLuint tx;
  GLuint fb;
  GLuint w;
  GLuint h;
} gltex_t;

struct gl_connector_t
{
  int tex_index;

  glsha_t shaders[3];
  gltex_t textures[10];
  glbuf_t vertexes[10];
} gl = {0};

void gl_errors(const char* place)
{
  GLenum error = 0;
  do
  {
    GLenum error = glGetError();
    if (error > GL_NO_ERROR)
      printf("GL Error at %s : %i\n", place, error);
  } while (error > GL_NO_ERROR);
}

glsha_t create_texture_shader()
{
  char* vsh =
      "#version 120\n"
      "attribute vec3 position;"
      "attribute vec3 texcoord;"
      "uniform mat4 projection;"
      "uniform sampler2D samplera;"
      "uniform sampler2D samplerb;"
      "varying vec3 vUv;"
      "void main ( )"
      "{"
      "    gl_Position = projection * vec4(position,1.0);"
      "    vUv = texcoord;"
      "}";

  char* fsh =
      "#version 120\n"
      "uniform sampler2D samplera;"
      "uniform sampler2D samplerb;"
      "varying vec3 vUv;"
      "void main( )"
      "{"
      "	if (vUv.z == 1.0)"
      "	{"
      "		gl_FragColor = texture2D(samplerb, vUv.xy);"
      "	}"
      "	else"
      "	{"
      "		gl_FragColor = texture2D(samplera, vUv.xy);"
      "	}"
      "}";

  return gl_shader_create(vsh,
                          fsh,
                          2,
                          ((const char*[]){"position", "texcoord"}),
                          3,
                          ((const char*[]){"projection", "samplera", "samplerb"}));
}

glsha_t create_color_shader()
{
  char* vsh =
      "#version 120\n"
      "attribute vec3 position;"
      "attribute vec3 texcoord;"
      "uniform mat4 projection;"
      "void main ( )"
      "{"
      "  gl_Position = projection * vec4(position,1.0);"
      "}";

  char* fsh =
      "#version 120\n"
      "void main( )"
      "{"
      "  gl_FragColor = vec4(0.0,0.0,0.0,1.0);"
      "}";

  return gl_shader_create(vsh,
                          fsh,
                          2,
                          ((const char*[]){"position", "texcoord"}),
                          1,
                          ((const char*[]){"projection"}));
}

glsha_t create_blur_shader()
{

  char* vsh =
      "#version 120\n"
      "attribute vec3 position;"
      "attribute vec3 texcoord;"
      "uniform mat4 projection;"
      "uniform sampler2D samplera;"
      "varying vec3 vUv;"
      "void main ( )"
      "{"
      "  gl_Position = projection * vec4(position,1.0);"
      "  vUv = texcoord;"
      "}";

  char* fsh =
      "#version 120\n"
      "uniform sampler2D samplera;\n"
      "varying vec3 vUv;"

      "void main()"
      "{"
      " float Pi = 6.28318530718;" // pi * 2

      " float Directions = 16.0;" // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
      " float Quality    = 4.0;"  // BLUR QUALITY (Default 4.0 - More is better but slower)
      " float Size       = 10.0;" // BLUR SIZE (Radius)
      " vec2 Radius = Size / vec2(4096,4096);"

      // Pixel colour
      " vec4 Color = texture2D(samplera, vUv.xy);"

      // Blur calculations
      " for (float d = 0.0; d < Pi; d += Pi / Directions)"
      " {"
      "  for (float i = 1.0 / Quality; i <= 1.0; i += 1.0 / Quality)"
      "  {"
      "   Color += texture2D(samplera, vUv.xy + vec2(cos(d), sin(d)) * Radius * i);"
      "  }"
      " }"
      // Output to screen
      " Color /= Quality * Directions;"
      " gl_FragColor = Color;"
      "}";

  return gl_shader_create(vsh,
                          fsh,
                          2,
                          ((const char*[]){"position", "texcoord"}),
                          2,
                          ((const char*[]){"projection", "samplera"}));
}

gltex_t gl_create_texture(uint32_t w, uint32_t h)
{
  gltex_t tex = {0};

  tex.index = gl.tex_index++;
  tex.w     = w;
  tex.h     = h;

  glGenTextures(1, &tex.tx);

  glActiveTexture(GL_TEXTURE0 + tex.index);
  glBindTexture(GL_TEXTURE_2D, tex.tx);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glGenFramebuffers(1, &tex.fb);

  glBindFramebuffer(GL_FRAMEBUFFER, tex.fb);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.tx, 0);

  return tex;
}

void gl_delete_texture(gltex_t tex)
{
  glDeleteTextures(1, &tex.tx);
  glDeleteFramebuffers(1, &tex.fb);
}

glbuf_t create_buffer()
{
  glbuf_t vb = {0};

  glGenBuffers(1, &vb.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vb.vbo);
  glGenVertexArrays(1, &vb.vao);
  glBindVertexArray(vb.vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (const GLvoid*)8);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vb;
}

void gl_delete_vertex_buffer(glbuf_t buf)
{
  glDeleteBuffers(1, &buf.vbo);
  glDeleteVertexArrays(1, &buf.vao);
}

void gl_init(width, height)
{
  glewInit();

  gl.shaders[SH_TEXTURE] = create_texture_shader();
  gl.shaders[SH_COLOR]   = create_color_shader();
  gl.shaders[SH_BLUR]    = create_blur_shader();

  /* texture 0 is preserved for context's default buffer */
  GLint context_fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &context_fb);
  gl.textures[0].fb = context_fb;

  /* buffer 0 is preserved for framebuffer drawing */
  gl.vertexes[0] = create_buffer();
  gl.vertexes[1] = create_buffer();
}

glrect_t gl_get_texture(uint32_t page, uint32_t w, uint32_t h)
{
  assert(page > 0); /* 0 is reserved for context's default framebuffer */

  if (gl.textures[page].w == 0)
  {
    int x = 256;
    int y = 256;
    while (x < w)
      x *= 2;
    while (y < h)
      y *= 2;
    gl.textures[page] = gl_create_texture(x, y);
  }

  gltex_t tex = gl.textures[page];

  return ((glrect_t){.w = tex.w, .h = tex.h});
}

void gl_update_vertexes(fb_t* fb)
{
  glBindBuffer(GL_ARRAY_BUFFER, gl.vertexes[1].vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fb->pos, fb->data, GL_DYNAMIC_DRAW);
  gl.vertexes[1].flo_buf = fb;
}

void gl_update_textures(int page, bm_t* bmp)
{
  glActiveTexture(GL_TEXTURE0 + gl.textures[page].index);

  if (bmp->w != gl.textures[page].w || bmp->h != gl.textures[page].h)
  {
    // resize texture and framebuffer
  }

  // when size is the same use subimage for speed
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->data);
}

void gl_clear_framebuffer(int page, float r, float g, float b, float a)
{
  glBindFramebuffer(GL_FRAMEBUFFER, gl.textures[page].fb);
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl_draw_rectangle(glrect_t src_reg, glrect_t tgt_reg)
{
  GLfloat data[] =
      {
          0.0,
          0.0,
          0.0,
          (float)src_reg.h / 4096.0,
          0.0,

          tgt_reg.w,
          tgt_reg.h,
          (float)src_reg.w / 4096.0,
          0.0,
          0.0,

          0.0,
          tgt_reg.h,
          0.0,
          0.0,
          0.0,

          0.0,
          0.0,
          0.0,
          (float)src_reg.h / 4096.0,
          0.0,

          tgt_reg.w,
          0.0,
          (float)src_reg.w / 4096.0,
          (float)src_reg.h / 4096.0,
          0.0,

          tgt_reg.w,
          tgt_reg.h,
          (float)src_reg.w / 4096.0,
          0.0,
          0.0,
      };

  glBindBuffer(GL_ARRAY_BUFFER, gl.vertexes[0].vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, data, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gl_draw_vertexes_in_framebuffer(int          page,
                                     int          start,
                                     int          end,
                                     glrect_t     reg_src,
                                     glrect_t     reg_tgt,
                                     gl_sha_typ_t shader)
{
  matrix4array_t projection;
  projection.matrix = m4_defaultortho(0.0, reg_src.w, reg_src.h, 0, 0.0, 1.0);

  glUseProgram(gl.shaders[shader].name);

  if (shader == SH_TEXTURE)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniformMatrix4fv(gl.shaders[shader].uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, reg_tgt.w, reg_tgt.h);

    glUniform1i(gl.shaders[shader].uni_loc[1], gl.textures[1].index);
    //glUniform1i(gl.shaders[shader].uni_loc[2], gl.textures[2].index);
  }
  else if (shader == SH_COLOR)
  {
    glUniformMatrix4fv(gl.shaders[shader].uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, reg_tgt.w, reg_tgt.h);
  }
  else if (shader == SH_BLUR)
  {
    glUniformMatrix4fv(gl.shaders[shader].uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, reg_tgt.w, reg_tgt.h);

    glUniform1i(gl.shaders[shader].uni_loc[1], gl.textures[1].index);
  }

  glBindVertexArray(gl.vertexes[1].vao);
  glBindFramebuffer(GL_FRAMEBUFFER, gl.textures[page].fb);

  glDrawArrays(GL_TRIANGLES, start, end - start);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
}

void gl_draw_framebuffer_in_framebuffer(int          src_page,
                                        int          tgt_page,
                                        glrect_t     src_reg,
                                        glrect_t     tgt_reg,
                                        glrect_t     window,
                                        gl_sha_typ_t shader)
{
  glUseProgram(gl.shaders[shader].name);

  if (shader == SH_TEXTURE)
  {
    glUniform1i(gl.shaders[shader].uni_loc[1], gl.textures[src_page].index);
  }
  else if (shader == SH_BLUR)
  {
    glUniform1i(gl.shaders[shader].uni_loc[1], gl.textures[src_page].index);
  }

  matrix4array_t projection;
  projection.matrix = m4_defaultortho(0.0, tgt_reg.w, tgt_reg.h, 0.0, 0.0, 1.0);
  glUniformMatrix4fv(gl.shaders[shader].uni_loc[0], 1, 0, projection.array);

  glViewport(0, 0, tgt_reg.w, tgt_reg.h);

  glBindFramebuffer(GL_FRAMEBUFFER, gl.textures[tgt_page].fb);
  glBindVertexArray(gl.vertexes[0].vao);

  if (window.w > 0)
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(window.x, tgt_reg.h - window.y - window.h, window.w, window.h); // force upside down
  }

  gl_draw_rectangle(src_reg, tgt_reg);

  glDisable(GL_SCISSOR_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* draw to texture page, mainly from ffmpeg */
void gl_draw_to_texture(int page, int w, int h, void* data)
{
  gltex_t texture = gl.textures[page];

  glActiveTexture(GL_TEXTURE0 + texture.index);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
}

#endif
