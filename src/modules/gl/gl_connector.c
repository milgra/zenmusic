/*
  OpenGL Connector Module for Zen Multimedia Desktop System
  Renders incoming triangles and textures to screen
  Composes framebuffers together

  gl_connector -> GPU
  
 */

#ifndef gl_connector_h
#define gl_connector_h

#include "gl_floatbuffer.c"
#include "mtbitmap.c"
#include "mtmath4.c"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum _glshader_t // texture loading state
{
  SH_TEXTURE,
  SH_COLOR,
  SH_BLUR
} glshader_t;

typedef struct _ver_buf_t
{
  GLuint vbo;
  GLuint vao;
  fb_t*  flo_buf;
} ver_buf_t;

typedef struct _region_t
{
  int x;
  int y;
  int w;
  int h;
} region_t;

void gl_init();
void gl_update_vertexes(fb_t* fb);
void gl_update_textures(bm_t* bmp);
void gl_clear_framebuffer(int index, float r, float g, float b, float a);
void gl_draw_vertexes_in_framebuffer(int        index,
                                     int        start,
                                     int        end,
                                     int        width,
                                     int        height,
                                     int        tgt_w,
                                     int        tht_h,
                                     region_t   region,
                                     glshader_t shader);
void gl_draw_framebuffer_in_framebuffer(int        src_ind,
                                        int        tgt_ind,
                                        int        src_w,
                                        int        src_h,
                                        int        width,
                                        int        height,
                                        region_t   region,
                                        glshader_t shader);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_utils.c"
#include "mtmath2.c"
#include <stdio.h>
#include <stdlib.h>

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
      " float Size       = 8.0;"  // BLUR SIZE (Radius)
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

ver_buf_t create_vertex_buffer()
{
  ver_buf_t vb = {0};

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

glsha_t texture_sh;
glsha_t color_sh;
glsha_t blur_sh;

gltex_t   textures[10] = {0};
ver_buf_t vertexes[10] = {0};

void gl_init(width, height)
{
  glewInit();

  // create shaders

  texture_sh = create_texture_shader();
  color_sh   = create_color_shader();
  blur_sh    = create_blur_shader();

  // create textures

  GLint def_fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &def_fb);

  textures[0].fb = def_fb;              // context's buffer for drawing
  textures[1]    = gl_create_texture(); // texture map
  textures[2]    = gl_create_texture(); // video texture
  textures[3]    = gl_create_texture(); // offscreen buffer
  textures[4]    = gl_create_texture(); // offscreen buffer
  textures[5]    = gl_create_texture(); // offscreen buffer
  textures[6]    = gl_create_texture(); // offscreen buffer

  // create vertex buffers

  vertexes[0] = create_vertex_buffer();
  vertexes[1] = create_vertex_buffer();
}

// update vertexes
void gl_update_vertexes(fb_t* fb)
{
  glBindBuffer(GL_ARRAY_BUFFER, vertexes[0].vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fb->pos, fb->data, GL_DYNAMIC_DRAW);
  vertexes[0].flo_buf = fb;
}

// update texture map
void gl_update_textures(bm_t* bmp)
{
  glActiveTexture(GL_TEXTURE0);

  if (bmp->w != textures[1].w || bmp->h != textures[1].h)
  {
    // resize texture and framebuffer
  }

  // when size is the same use subimage for speed
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->data);
}

void gl_clear_framebuffer(int index, float r, float g, float b, float a)
{
  // 0 - context frame buffer
  // > 1 - other frame buffers
  glBindFramebuffer(GL_FRAMEBUFFER, textures[index].fb);
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// draw vertexes into selected framebuffer
void gl_draw_vertexes_in_framebuffer(int        index,
                                     int        start,
                                     int        end,
                                     int        width,
                                     int        height,
                                     int        tgt_w,
                                     int        tgt_h,
                                     region_t   region,
                                     glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    glUseProgram(texture_sh.name);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(texture_sh.uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, tgt_w, tgt_h);

    //glScissor(200, 200, 100, 100);
    //glEnable(GL_SCISSOR_TEST);

    // set textures
    glUniform1i(texture_sh.uni_loc[1], 0);
    glUniform1i(texture_sh.uni_loc[2], 1);
  }
  else if (shader == SH_COLOR)
  {
    glUseProgram(color_sh.name);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(color_sh.uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, tgt_w, tgt_h);
  }
  else if (shader == SH_BLUR)
  {
    glUseProgram(blur_sh.name);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(blur_sh.uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, tgt_w, tgt_h);

    // set textures
    glUniform1i(blur_sh.uni_loc[1], 0);
  }

  glBindVertexArray(vertexes[0].vao);
  glBindFramebuffer(GL_FRAMEBUFFER, textures[index].fb);

  glDrawArrays(GL_TRIANGLES, start, end - start);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
}

void gl_draw_framebuffer_in_framebuffer(int        src_ind,
                                        int        tgt_ind,
                                        int        src_w,
                                        int        src_h,
                                        int        width,
                                        int        height,
                                        region_t   region,
                                        glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    glUseProgram(texture_sh.name);
    glUniform1i(texture_sh.uni_loc[1], textures[src_ind].index);
  }
  else if (shader == SH_BLUR)
  {
    glUseProgram(blur_sh.name);
    glUniform1i(blur_sh.uni_loc[1], textures[src_ind].index);
  }

  GLfloat data[] = {
      0.0,
      0.0,
      0.0,
      (float)src_h / 4096,
      0.0,

      width,
      height,
      (float)src_w / 4096,
      0.0,
      0.0,

      0.0,
      height,
      0.0,
      0.0,
      0.0,

      0.0,
      0.0,
      0.0,
      (float)src_h / 4096,
      0.0,

      width,
      0.0,
      (float)src_w / 4096,
      (float)src_h / 4096,
      0.0,

      width,
      height,
      (float)src_w / 4096,
      0.0,
      0.0,
  };

  glBindBuffer(GL_ARRAY_BUFFER, vertexes[1].vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, data, GL_DYNAMIC_DRAW);

  matrix4array_t projection;
  projection.matrix = m4_defaultortho(0.0, width, height, 0.0, 0.0, 1.0);
  glUniformMatrix4fv(texture_sh.uni_loc[0], 1, 0, projection.array);

  glViewport(0, 0, width, height);

  glBindFramebuffer(GL_FRAMEBUFFER, textures[tgt_ind].fb);
  glBindVertexArray(vertexes[1].vao);

  if (region.w > 0)
  {
    glEnable(GL_SCISSOR_TEST);
    // upside down
    glScissor(region.x, height - region.y - region.h, region.w, region.h);
  }

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisable(GL_SCISSOR_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// blur selected framebuffer
void gl_blur_framebuffer(int index)
{
}

#endif
