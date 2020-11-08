#ifndef gl_utils_h
#define gl_utils_h

#include <GL/glew.h>

typedef struct _gltex_t
{
  GLuint tx;
  GLuint fb;
  GLuint index;
} gltex_t;

GLuint gl_shader_create(const char*  vertex_source,
                        const char*  fragment_source,
                        int          attribute_locations_length,
                        const char** attribute_structure,
                        int          uniform_locations_length,
                        const char** uniform_structure,
                        GLint*       uniform_locations);

gltex_t gl_create_texture();
void    gl_errors(const char* place);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <stdio.h>

int tex_index = 0;

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

  return program;
}

gltex_t gl_create_texture(int w, int h)
{
  gltex_t tex;

  tex.index = tex_index++;

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

#endif
