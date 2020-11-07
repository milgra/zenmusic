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
  SH_BLACK,
  SH_TEXTURE,
  SH_BLUR
} glshader_t;

void gl_init();
void gl_resize(int width, int height);
void gl_update_vertexes(fb_t* fb);
void gl_update_textures(bm_t* bmp);
void gl_draw_vertexes_in_framebuffer(int index, int start, int end, v4_t region, glshader_t shader);
void gl_clear_framebuffer(int index);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmath2.c"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

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

GLuint gl_shader_compile(GLenum type, const GLchar* source)
{
  GLint  status, logLength, realLength;
  GLuint shader = 0;

  status = 0;
  shader = glCreateShader(type);

  if (shader > 0)
  {
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
      GLchar log[logLength];

      glGetShaderInfoLog(shader,
                         logLength,
                         &realLength,
                         log);

      printf("Shader compile log: %s\n", log);
    }

    glGetShaderiv(shader,
                  GL_COMPILE_STATUS,
                  &status);

    if (status != GL_TRUE)
      return 0;
  }
  else
    printf("Cannot create shader\n");

  return shader;
}

int gl_shader_link(GLuint program)
{
  GLint status, logLength, realLength;

  glLinkProgram(program);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

  if (logLength > 0)
  {
    GLchar log[logLength];
    glGetProgramInfoLog(program, logLength, &realLength, log);
    printf("Program link log : %i %s\n", realLength, log);
  }

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_TRUE)
    return 1;
  return 0;
}

GLuint gl_shader_create(const char*  vertex_source,
                        const char*  fragment_source,
                        int          attribute_locations_length,
                        const char** attribute_structure,
                        int          uniform_locations_length,
                        const char** uniform_structure,
                        GLint*       uniform_locations)
{

  GLuint program = glCreateProgram();

  GLuint vertex_shader = gl_shader_compile(GL_VERTEX_SHADER, vertex_source);
  if (vertex_shader == 0) printf("Failed to compile vertex shader : %s\n", vertex_source);

  GLuint fragment_shader = gl_shader_compile(GL_FRAGMENT_SHADER, fragment_source);
  if (fragment_shader == 0) printf("Failed to compile fragment shader : %s\n", fragment_source);

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  for (int index = 0; index < attribute_locations_length; index++)
  {
    const GLchar* name = attribute_structure[index];
    glBindAttribLocation(program, index, name);
  }

  int success = gl_shader_link(program);

  if (success == 1)
  {
    for (int index = 0; index < uniform_locations_length; index++)
    {
      const GLchar* name       = uniform_structure[index];
      GLint         location   = glGetUniformLocation(program, name);
      uniform_locations[index] = location;
    }
  }
  else
    printf("Failed to link shader program\n");

  if (vertex_shader > 0)
  {
    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);
  }

  if (fragment_shader > 0)
  {
    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);
  }

#ifdef DEBUG
  gl_errors("after gl_shader_create\n");
#endif

  return program;
}

char* texture_vsh =
#include "texture.vsh"
    ;
char* texture_fsh =
#include "texture.fsh"
    ;

char* color_vsh =
#include "color.vsh"
    ;
char* color_fsh =
#include "color.fsh"
    ;

char* blur_vsh =
#include "blur.vsh"
    ;

char* blur_fsh =
#include "blur.fsh"
    ;

GLint unif_name_texture[3];
GLint unif_name_color[1];
GLint unif_name_blur[3];

GLuint texture_sh;
GLuint color_sh;
GLuint blur_sh;

int context_w;
int context_h;

fb_t* floatbuffer;
int   framebuffers[10];

void gl_init(width, height)
{
  glewInit();

  texture_sh = gl_shader_create(texture_vsh,
                                texture_fsh,
                                2,
                                ((const char*[]){"position", "texcoord"}),
                                3,
                                ((const char*[]){"projection", "samplera", "samplerb"}),
                                unif_name_texture);

  color_sh = gl_shader_create(color_vsh,
                              color_fsh,
                              2,
                              ((const char*[]){"position", "texcoord"}),
                              1,
                              ((const char*[]){"projection"}),
                              unif_name_color);

  blur_sh = gl_shader_create(blur_vsh,
                             blur_fsh,
                             2,
                             ((const char*[]){"position", "texcoord"}),
                             1,
                             ((const char*[]){"projection", "samplera"}),
                             unif_name_blur);

  // create vertex buffer
  GLuint vbuffer_name_u;

  glGenBuffers(1, &vbuffer_name_u);
  glBindBuffer(GL_ARRAY_BUFFER, vbuffer_name_u);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (const GLvoid*)8);

  GLuint texture_name_a;

  glGenTextures(1, &texture_name_a);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_name_a);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4096, 4096, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  GLuint texture_name_b;

  glGenTextures(1, &texture_name_b);

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, texture_name_b);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4096, 4096, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glClearColor(0.5, 0.5, 0.5, 1.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  int contextFrameBuffer;
  int contextRenderBuffer;

  glGetIntegerv(
      GL_FRAMEBUFFER_BINDING,
      &contextFrameBuffer);

  glGetIntegerv(
      GL_RENDERBUFFER_BINDING,
      &contextRenderBuffer);

  framebuffers[0] = contextFrameBuffer;
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
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// draw vertexes into selected framebuffer
void gl_draw_vertexes_in_framebuffer(int index, int start, int end, v4_t region, glshader_t shader)
{

  if (shader == SH_TEXTURE)
  {
    // use full context for projection
    matrix4array_t projection;
    projection.matrix = m4_defaultortho(0.0, context_w, context_h, 0, 0.0, 1.0);

    glUseProgram(texture_sh);

    // set projection and viewport
    glUniformMatrix4fv(unif_name_texture[0], 1, 0, projection.array);
    glViewport(0, 0, context_w, context_h);

    //glScissor(200, 200, 100, 100);
    //glEnable(GL_SCISSOR_TEST);

    // set textures
    glUniform1i(unif_name_texture[1], 0);
    glUniform1i(unif_name_texture[2], 1);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
  glDrawArrays(GL_TRIANGLES, 0, floatbuffer->pos / 5);
}

void gl_draw_framebuffer_in_framebuffer(int src_ind, int tgt_ind, glshader_t shader)
{
}

// blur selected framebuffer
void gl_blur_framebuffer(int index)
{
}

#endif
