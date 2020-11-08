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

void gl_init();
void gl_resize(int width, int height);
void gl_update_vertexes(fb_t* fb);
void gl_update_textures(bm_t* bmp);
void gl_draw_vertexes_in_framebuffer(int index, int start, int end, v4_t region, glshader_t shader);
void gl_clear_framebuffer(int index);
void gl_draw_framebuffer_in_framebuffer(int src_ind, int tgt_ind, glshader_t shader);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_utils.c"
#include "mtmath2.c"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

GLuint create_texture_shader(GLint* uniforms)
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
                          ((const char*[]){"projection", "samplera", "samplerb"}),
                          uniforms);
}

GLuint create_color_shader(GLint* uniforms)
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
                          ((const char*[]){"projection"}),
                          uniforms);
}

GLuint create_blur_shader(GLint* uniforms)
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
                          ((const char*[]){"projection", "samplera"}),
                          uniforms);
}

GLint unif_name_texture[3];
GLint unif_name_color[1];
GLint unif_name_blur[2];

GLuint texture_sh;
GLuint color_sh;
GLuint blur_sh;

int context_w;
int context_h;
int context_fb;

fb_t*   floatbuffer;
gltex_t textures[10] = {0};

GLuint ver_arr_a;
GLuint ver_arr_b;
GLuint ver_buf_a;
GLuint ver_buf_b;

void gl_init(width, height)
{
  glewInit();

  // create shaders

  texture_sh = create_texture_shader(unif_name_texture);
  color_sh   = create_color_shader(unif_name_color);
  blur_sh    = create_blur_shader(unif_name_blur);

  // create textures

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &context_fb);

  textures[0].fb = context_fb;                    // context's buffer for drawing
  textures[1]    = gl_create_texture(4096, 4096); // texture map
  textures[2]    = gl_create_texture(4096, 4096); // video texture
  textures[3]    = gl_create_texture(4096, 4096); // offscreen buffer

  // create vertex buffers

  glGenBuffers(1, &ver_buf_a);
  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_a);
  glGenVertexArrays(1, &ver_arr_a);
  glBindVertexArray(ver_arr_a);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (const GLvoid*)8);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &ver_buf_b);
  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_b);
  glGenVertexArrays(1, &ver_arr_b);
  glBindVertexArray(ver_arr_b);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (const GLvoid*)8);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glClearColor(0.0, 0.0, 0.0, 1.0);
}

void gl_resize(int width, int height)
{
  printf("gl resize %i %i\n", width, height);

  context_w = width;
  context_h = height;
}

// update vertexes
void gl_update_vertexes(fb_t* fb)
{
  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_a);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fb->pos, fb->data, GL_DYNAMIC_DRAW);
  floatbuffer = fb;
}

// update texture map
void gl_update_textures(bm_t* bmp)
{
  glActiveTexture(GL_TEXTURE0);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->data);
}

void gl_clear_framebuffer(int index)
{
  // 0 - context frame buffer
  // > 1 - other frame buffers
  glBindFramebuffer(GL_FRAMEBUFFER, textures[index].fb);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// draw vertexes into selected framebuffer
void gl_draw_vertexes_in_framebuffer(int index, int start, int end, v4_t region, glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    glUseProgram(texture_sh);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, context_w, context_h, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(unif_name_texture[0], 1, 0, projection.array);
    glViewport(0, 0, context_w, context_h);

    //glScissor(200, 200, 100, 100);
    //glEnable(GL_SCISSOR_TEST);

    // set textures
    glUniform1i(unif_name_texture[1], 0);
    glUniform1i(unif_name_texture[2], 1);
  }
  else if (shader == SH_COLOR)
  {
    glUseProgram(color_sh);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, context_w, context_h, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(unif_name_color[0], 1, 0, projection.array);
    glViewport(0, 0, context_w, context_h);
  }
  else if (shader == SH_BLUR)
  {
    glUseProgram(blur_sh);
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, context_w, context_h, 0, 0.0, 1.0);

    // set projection and viewport
    glUniformMatrix4fv(unif_name_blur[0], 1, 0, projection.array);
    glViewport(0, 0, context_w, context_h);

    // set textures
    glUniform1i(unif_name_blur[1], 0);
  }

  //glScissor(200, 200, 100, 100);
  //glEnable(GL_SCISSOR_TEST);

  glBindVertexArray(ver_arr_a);
  glBindFramebuffer(GL_FRAMEBUFFER, textures[index].fb);

  glDrawArrays(GL_TRIANGLES, 0, floatbuffer->pos / 5);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
}

void gl_draw_framebuffer_in_framebuffer(int src_ind, int tgt_ind, glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    glUseProgram(texture_sh);
    glUniform1i(unif_name_texture[1], textures[src_ind].index);
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

  glBindBuffer(GL_ARRAY_BUFFER, ver_buf_b);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, data, GL_DYNAMIC_DRAW);

  matrix4array_t projection;
  projection.matrix = m4_defaultortho(0.0, context_w, context_h, 0, 0.0, 1.0);
  glUniformMatrix4fv(unif_name_texture[0], 1, 0, projection.array);

  glViewport(0, 0, context_w, context_h);

  glBindFramebuffer(GL_FRAMEBUFFER, textures[tgt_ind].fb);
  glBindVertexArray(ver_arr_b);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// blur selected framebuffer
void gl_blur_framebuffer(int index)
{
}

#endif
