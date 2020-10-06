#include "glew.h"
#include "math2.c"
#include "math4.c"
#include "modules/gl_connector.c"
#include "modules/wm_connector.c"
#include "mtbmp.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* blend_vsh =
#include "shaders/blend.vsh"
  ;
char* blend_fsh =
#include "shaders/blend.fsh"
  ;

void
main_init(const char* mit, int width, int height)
{
  printf("main init\n");

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  const char* uniforms_blend[] = { "2", "projection", "texture" };
  const char* attributes_blend[] = { "2", "position", "texcoord" };

  glewInit();

  GLint uniform_name_a[2];

  GLuint shader_name_i = ogl_shader_create(
    blend_vsh, blend_fsh, uniforms_blend, attributes_blend, uniform_name_a);

  glUseProgram(shader_name_i);

  printf("shader created %i", shader_name_i);

  v2_t dimensions = { .x = width, .y = height };

  m4_t matrix =
    m4_defaultortho(0.0, dimensions.x, -dimensions.y, 0.0, 0.0, 1.0);

  matrix4array_t projection;
  projection.matrix = matrix;

  glUniformMatrix4fv(uniform_name_a[0], 1, 0, projection.array);

  // create vertex buffer
  GLuint vbuffer_name_u;

  glGenBuffers(1, &vbuffer_name_u);
  glBindBuffer(GL_ARRAY_BUFFER, vbuffer_name_u);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (const GLvoid*)12);

  GLuint texture_name_u;

  glGenTextures(1, &texture_name_u);
  glBindTexture(GL_TEXTURE_2D, texture_name_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_name_u);
  glUniform1i(uniform_name_a[1], 0);

  glClearColor(0.5, 0.5, 0.5, 1.0);

  mtbmp_t* texture_bmp = mtbmp_alloc(1024, 1024);
  mtbmp_fill_with_color(texture_bmp, 0, 0, 1024, 1023, 0xFF0000FF);

  glTexSubImage2D(GL_TEXTURE_2D,
                  0,
                  0,
                  0,
                  1024,
                  1024,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  texture_bmp->bytes);

  // create and upload rectangle

  GLfloat vertexes[] = {

    0.0,    0.0,     0.0, 0.0f, 0.0f, 1024.0, 0.0,     0.0, 1.0f, 0.0f,
    0.0,    -1024.0, 0.0, 0.0f, 1.0f, 1024.0, 0.0,     0.0, 1.0f, 0.0f,
    1024.0, -1024.0, 0.0, 1.0f, 1.0f, 0.0,    -1024.0, 0.0, 0.0f, 1.0f

  };

  glBufferData(
    GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, vertexes, GL_DYNAMIC_DRAW);

  SDL_StartTextInput();
};

void
main_loop(SDL_Window* window)
{

  char drag = 0;
  char quit = 0;
  SDL_Event event;

  while (!quit) {

    while (SDL_PollEvent(&event) != 0) {

      printf("pollevent\n");

      if (event.type == SDL_MOUSEBUTTONDOWN ||
          event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION) {

        int x = 0, y = 0;
        SDL_GetMouseState(&x, &y);

        // v2_t dimensions = { .x = x * scale, .y = y * scale };

        if (event.type == SDL_MOUSEBUTTONDOWN) {
          drag = 1;
          printf("mouse button down");
        } else if (event.type == SDL_MOUSEBUTTONUP) {
          drag = 0;
          printf("mouse button up");
        } else if (event.type == SDL_MOUSEMOTION && drag == 1) {
          printf("mouse dragged");
        }

      } else if (event.type == SDL_QUIT) {
        printf("sdl quit");
        quit = 1;
      } else if (event.type == SDL_WINDOWEVENT) {

        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {

          int32_t width = event.window.data1;
          int32_t height = event.window.data2;

          printf("window resized %i %i", width, height);
        }

      } else if (event.type == SDL_KEYDOWN) {

      } else if (event.type == SDL_TEXTINPUT) {
        printf("TEXT %s", event.text.text);
      } else if (event.type == SDL_APP_WILLENTERFOREGROUND) {
        printf("FOREGROUND");
      }

      glClearColor(0.4, 0.4, 0.4, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    SDL_GL_SwapWindow(window);
  }
}

void
main_free()
{
  SDL_StopTextInput();
};

int
main(int argc, char* args[])
{

  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  } else {
    printf("SDL Init Success\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_DisplayMode displaymode;
    SDL_GetCurrentDisplayMode(0, &displaymode);

    int32_t width = displaymode.w;
    int32_t height = displaymode.h;

    SDL_Window* window =
      SDL_CreateWindow("Zen Music",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       width,
                       height,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                         SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
      printf("Window Init Success\n");

      SDL_GLContext* context = SDL_GL_CreateContext(window);
      if (context == NULL) {
        printf("Context could not be created! SDL_Error: %s\n", SDL_GetError());
      } else {
        printf("Context Init Success\n");

        int nw;
        int nh;

        SDL_GL_GetDrawableSize(window, &nw, &nh);

        float scale = nw / width;

        printf("Scaling will be %f\n", scale);

        if (SDL_GL_SetSwapInterval(1) < 0)
          printf("SDL swap interval error %s\n", SDL_GetError());

        printf("Starting Zen Music\n");

        main_init("fasz", width, height);
        main_loop(window);
        main_free();

        SDL_GL_DeleteContext(context);
      }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
  }
}
