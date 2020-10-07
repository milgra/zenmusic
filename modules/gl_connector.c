/*
  OpenGL Connector Module for Zen Multimedia Desktop System

  events -> ui_generator -> ui_connector -> ui_compositor -> gl_connector -> GPU

 */

#ifndef gl_connector_h
#define gl_connector_h

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

void gl_init();
void gl_render();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "math2.c"
#include "math4.c"
#include "mtbm.c"
#include "mtfb.c"
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

/* internal : compile shader */

GLuint gl_shader_compile(GLenum type, const GLchar* source)
{
  GLint status, logLength, realLength;
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
      glGetShaderInfoLog(shader, logLength, &realLength, log);
      printf("Shader compile log: %s\n", log);
    }

    // get status

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
      return 0;
  }
  else
    printf("Cannot create shader\n");

  return shader;
}

/* internal : link shaders together in gpu */

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

GLuint gl_shader_create(const char* vertex_source,
                        const char* fragment_source,
                        const char** uniform_structure,
                        const char** attribute_structure,
                        GLint* uniform_locations)
{

  int uniform_locations_length = atoi(uniform_structure[0]);
  int attribute_locations_length = atoi(attribute_structure[0]);

  GLuint program = glCreateProgram();

  GLuint vertex_shader = gl_shader_compile(GL_VERTEX_SHADER, vertex_source);
  if (vertex_shader == 0) printf("Failed to compile vertex shader : %s\n", vertex_source);

  GLuint fragment_shader = gl_shader_compile(GL_FRAGMENT_SHADER, fragment_source);
  if (fragment_shader == 0) printf("Failed to compile fragment shader : %s\n", fragment_source);

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  for (int index = 0; index < attribute_locations_length; index++)
  {
    const GLchar* name = attribute_structure[index + 1];
    glBindAttribLocation(program, index, name);
  }

  int success = gl_shader_link(program);

  if (success == 1)
  {
    for (int index = 0; index < uniform_locations_length; index++)
    {
      const GLchar* name = uniform_structure[index + 1];
      GLint location = glGetUniformLocation(program, name);
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

char* blend_vsh =
#include "shaders/blend.vsh"
    ;
char* blend_fsh =
#include "shaders/blend.fsh"
    ;

void gl_init(width, height)
{

  const char* uniforms_blend[] = {"2", "projection", "texture"};
  const char* attributes_blend[] = {"2", "position", "texcoord"};

  glewInit();

  GLint uniform_name_a[2];

  GLuint shader_name_i = gl_shader_create(blend_vsh, blend_fsh,
                                          uniforms_blend,
                                          attributes_blend,
                                          uniform_name_a);

  glUseProgram(shader_name_i);

  m4_t matrix = m4_defaultortho(0.0, width, height, 0, 0.0, 1.0);

  matrix4array_t projection;
  projection.matrix = matrix;

  glUniformMatrix4fv(uniform_name_a[0], 1, 0, projection.array);

  // create vertex buffer
  GLuint vbuffer_name_u;

  glGenBuffers(1, &vbuffer_name_u);
  glBindBuffer(GL_ARRAY_BUFFER, vbuffer_name_u);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)8);

  GLuint texture_name_u;

  glGenTextures(1, &texture_name_u);
  glBindTexture(GL_TEXTURE_2D, texture_name_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_name_u);
  glUniform1i(uniform_name_a[1], 0);
  glClearColor(0.5, 0.5, 0.5, 1.0);
}

void gl_render(fb_t* fb, bm_t* bmp)
{
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->data);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fb->pos, fb->data, GL_DYNAMIC_DRAW);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, fb->pos / 4);
}

#endif
