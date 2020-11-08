/*
  OpenGL Connector Module for Zen Multimedia Desktop System
  Renders incoming triangles and textures to screen

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

void gl_init();
void gl_update_vertexes(fb_t* fb);
void gl_update_textures(bm_t* bmp);
void gl_clear_framebuffer(int index, float r, float g, float b, float a);
void gl_draw_vertexes_in_framebuffer(int        index,
                                     int        start,
                                     int        end,
                                     int        width,
                                     int        height,
                                     v4_t       region,
                                     glshader_t shader);
void gl_draw_framebuffer_in_framebuffer(int        src_ind,
                                        int        tgt_ind,
                                        int        width,
                                        int        height,
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
      "uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308) ;"
      "uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703) ;"
      "void main( )"
      "{"
      "  gl_FragColor = texture2D(samplera, vUv.xy / 1024.0) * weight[0] ;"
      "  for (int i=1; i<3; i++)"
      "  {"
      "  gl_FragColor += texture2D(samplera, (vUv.xy + vec2(0.0, offset[i])) / 1024.0) * weight[i];"
      "  gl_FragColor += texture2D(samplera, (vUv.xy - vec2(0.0, offset[i])) / 1024.0) * weight[i];"
      "  }"
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

gltex_t textures[10] = {0};

ver_buf_t ver_buf_a;
ver_buf_t ver_buf_b;

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

  // create vertex buffers

  ver_buf_a = create_vertex_buffer();
  ver_buf_b = create_vertex_buffer();
}

// update vertexes
void gl_update_vertexes(fb_t* fb)
{
  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_a.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fb->pos, fb->data, GL_DYNAMIC_DRAW);
  ver_buf_a.flo_buf = fb;
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
                                     v4_t       region,
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
    glViewport(0, 0, width, height);

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
    glViewport(0, 0, width, height);
  }
  else if (shader == SH_BLUR)
  {
    glUseProgram(blur_sh.name);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(blur_sh.uni_loc[0], 1, 0, projection.array);
    glViewport(0, 0, width, height);

    // set textures
    glUniform1i(blur_sh.uni_loc[1], 0);
  }

  //glScissor(200, 200, 100, 100);
  //glEnable(GL_SCISSOR_TEST);

  glBindVertexArray(ver_buf_a.vao);
  glBindFramebuffer(GL_FRAMEBUFFER, textures[index].fb);

  glDrawArrays(GL_TRIANGLES, 0, ver_buf_a.flo_buf->pos / 5);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
}

void gl_draw_framebuffer_in_framebuffer(int        src_ind,
                                        int        tgt_ind,
                                        int        width,
                                        int        height,
                                        glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    glUseProgram(texture_sh.name);
    glUniform1i(texture_sh.uni_loc[1], textures[src_ind].index);
  }

  GLfloat data[] = {
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,

      1024.0,
      1024.0,
      1.0,
      1.0,
      0.0,

      0.0,
      1024.0,
      0.0,
      1.0,
      0.0,

      0.0,
      0.0,
      0.0,
      0.0,
      0.0,

      1024.0,
      0.0,
      1.0,
      0.0,
      0.0,

      1024.0,
      1024.0,
      1.0,
      1.0,
      0.0,
  };

  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_b.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, data, GL_DYNAMIC_DRAW);

  matrix4array_t projection;
  projection.matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);
  glUniformMatrix4fv(texture_sh.uni_loc[0], 1, 0, projection.array);

  glViewport(0, 0, width, height);

  glBindFramebuffer(GL_FRAMEBUFFER, textures[tgt_ind].fb);
  glBindVertexArray(ver_buf_b.vao);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// blur selected framebuffer
void gl_blur_framebuffer(int index)
{
}

#endif
