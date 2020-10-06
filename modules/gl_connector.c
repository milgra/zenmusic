#ifndef gl_connector_h
#define gl_connector_h

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

GLuint
ogl_shader_create(const char* vertex_source,
                  const char* fragment_source,
                  const char** uniform_structure,
                  const char** attribute_structure,
                  GLint* uniform_locations);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

void
ogl_errors(const char* place)
{
  GLenum error = 0;
  do {
    GLenum error = glGetError();
    if (error > GL_NO_ERROR)
      printf("GL Error at %s : %i\n", place, error);
  } while (error > GL_NO_ERROR);
}

/* internal : compile shader */

GLuint
ogl_shader_compile(GLenum type, const GLchar* source)
{
  GLint status, logLength, realLength;
  GLuint shader = 0;

  status = 0;
  shader = glCreateShader(type);

  if (shader > 0) {
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
      GLchar log[logLength];
      glGetShaderInfoLog(shader, logLength, &realLength, log);
      printf("Shader compile log: %s\n", log);
    }

    // get status

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
      return 0;
  } else
    printf("Cannot create shader\n");

  return shader;
}

/* internal : link shaders together in gpu */

int
ogl_shader_link(GLuint program)
{
  GLint status, logLength, realLength;

  glLinkProgram(program);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

  if (logLength > 0) {
    GLchar log[logLength];
    glGetProgramInfoLog(program, logLength, &realLength, log);
    printf("Program link log : %i %s\n", realLength, log);
  }

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_TRUE)
    return 1;
  return 0;
}

GLuint
ogl_shader_create(const char* vertex_source,
                  const char* fragment_source,
                  const char** uniform_structure,
                  const char** attribute_structure,
                  GLint* uniform_locations)
{

  printf("create shader");

  int uniform_locations_length = atoi(uniform_structure[0]);
  int attribute_locations_length = atoi(attribute_structure[0]);

  GLuint program = glCreateProgram();

  GLuint vertex_shader = ogl_shader_compile(GL_VERTEX_SHADER, vertex_source);
  if (vertex_shader == 0)
    printf("Failed to compile vertex shader : %s\n", vertex_source);

  GLuint fragment_shader =
    ogl_shader_compile(GL_FRAGMENT_SHADER, fragment_source);
  if (fragment_shader == 0)
    printf("Failed to compile fragment shader : %s\n", fragment_source);

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  for (int index = 0; index < attribute_locations_length; index++) {
    const GLchar* name = attribute_structure[index + 1];
    glBindAttribLocation(program, index, name);
  }

  int success = ogl_shader_link(program);

  if (success == 1) {
    for (int index = 0; index < uniform_locations_length; index++) {
      const GLchar* name = uniform_structure[index + 1];
      GLint location = glGetUniformLocation(program, name);
      uniform_locations[index] = location;
    }
  } else
    printf("Failed to link shader program\n");

  if (vertex_shader > 0) {
    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);
  }

  if (fragment_shader > 0) {
    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);
  }

#ifdef DEBUG
  ogl_errors("after ogl_shader_create\n");
#endif

  return program;
}

#endif
