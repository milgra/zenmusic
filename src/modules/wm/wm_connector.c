/*
  Window Manager Connector Module for Zen Multimedia Desktop System
  Creates window and listens for events
  
  Currently it is SDL2
 */

#ifndef wm_connector_h
#define wm_connector_h

#include "wm_event.c"

void wm_init(void (*init)(int, int), void (*update)(ev_t), void (*render)(), void (*destroy)());

#endif

#if __INCLUDE_LEVEL__ == 0

#include "wm_event.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCROLL_RELEASED_DELAY 15         // if two scroll events arrive between this amount of millisecs scroll considered released
#define SCROLL_RELEASED_SLOWDOWN 0.999   // slowdown ratio of released scrolling
#define SCROLL_TOUCHED_SLOWDOWN 0.8      // slowdown ratio of touched scrolling
#define SCROLL_TOUCHED_TIMEOUT 300       // when to stop scrolling after last pressed touch scroll event
#define SCROLL_SLOWDOWN_MULTIPLIER 0.995 // slowdown ratio is multiplied by this value with every step to produce a parabolic slowdown
#define SCROLL_MINIMUM_DELTA 0.1         // scroll events aren't dispatched under this scroll delta value

void wm_init(void (*init)(int, int),
             void (*update)(ev_t),
             void (*render)(),
             void (*destroy)())
{
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }
  else
  {
    printf("SDL Init Success\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_DisplayMode displaymode;
    SDL_GetCurrentDisplayMode(0, &displaymode);

    int32_t width  = displaymode.w;
    int32_t height = displaymode.h;

    SDL_Window* window = SDL_CreateWindow("Zen Music",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          width,
                                          height,
                                          SDL_WINDOW_OPENGL |
                                              SDL_WINDOW_SHOWN |
                                              SDL_WINDOW_ALLOW_HIGHDPI |
                                              SDL_WINDOW_RESIZABLE);

    if (window == NULL)
    {
      printf("SDL Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
      printf("SDL Window Init Success\n");

      SDL_GLContext* context = SDL_GL_CreateContext(window);
      if (context == NULL)
      {
        printf("SDL Context could not be created! SDL_Error: %s\n", SDL_GetError());
      }
      else
      {
        printf("SDL Context Init Success\n");

        int nw;
        int nh;

        SDL_GL_GetDrawableSize(window, &nw, &nh);

        float scale = nw / width;

        printf("SDL Scaling will be %f\n", scale);

        if (SDL_GL_SetSwapInterval(1) < 0) printf("SDL swap interval error %s\n", SDL_GetError());

        SDL_StartTextInput();

        (*init)(width, height);

        char      quit = 0;
        ev_t      ev   = {0}; // zen event
        SDL_Event event;
        uint32_t  lastticks = SDL_GetTicks();

        struct scroll_t
        {
          char     active;
          float    sx;
          float    sy;
          float    slowdown;
          uint32_t time_to_stop;
          uint32_t time_last;
        } scroll = {0};

        while (!quit)
        {
          ev.type = EV_EMPTY;
          ev.time = SDL_GetTicks();

          while (SDL_PollEvent(&event) != 0)
          {
            if (event.type == SDL_MOUSEBUTTONDOWN ||
                event.type == SDL_MOUSEBUTTONUP ||
                event.type == SDL_MOUSEMOTION)
            {

              SDL_GetMouseState(&ev.x, &ev.y);

              if (event.type == SDL_MOUSEBUTTONDOWN)
              {
                ev.type = EV_MDOWN;
                ev.drag = 1;
              }
              else if (event.type == SDL_MOUSEBUTTONUP)
              {
                ev.type = EV_MUP;
                ev.drag = 0;
              }
              else if (event.type == SDL_MOUSEMOTION)
              {
                ev.dx   = event.motion.xrel;
                ev.dy   = event.motion.yrel;
                ev.type = EV_MMOVE;
              }
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {

              scroll.active = 1;

              uint32_t delta   = ev.time - scroll.time_last;
              scroll.time_last = ev.time;

              scroll.sx += (float)event.wheel.x * 2.0;
              scroll.sy += (float)event.wheel.y * 2.0;

              scroll.time_to_stop = delta < SCROLL_RELEASED_DELAY ? UINT32_MAX : ev.time + SCROLL_TOUCHED_TIMEOUT;
              scroll.slowdown     = delta < SCROLL_RELEASED_DELAY ? SCROLL_RELEASED_SLOWDOWN : SCROLL_TOUCHED_SLOWDOWN;
            }
            else if (event.type == SDL_QUIT)
            {
              quit = 1;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
              if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              {
                ev.type = EV_RESIZE;
                ev.w    = event.window.data1;
                ev.h    = event.window.data2;
              }
            }
            else if (event.type == SDL_KEYDOWN)
            {
            }
            else if (event.type == SDL_TEXTINPUT)
            {
              ev.type = EV_TEXT;
              ev.text = event.text.text;
            }

            (*update)(ev);
          }

          if (scroll.active)
          {
            if (ev.time < scroll.time_to_stop &&
                (fabs(scroll.sx) > SCROLL_MINIMUM_DELTA ||
                 fabs(scroll.sy) > SCROLL_MINIMUM_DELTA))
            {
              ev.type = EV_SCROLL;
              ev.dx   = scroll.sx;
              ev.dy   = scroll.sy;
              scroll.slowdown *= SCROLL_SLOWDOWN_MULTIPLIER;
              scroll.sx *= scroll.slowdown;
              scroll.sy *= scroll.slowdown;
              (*update)(ev);
            }
            else
            {
              ev.type       = EV_SCROLL;
              ev.dx         = 0.0;
              ev.dy         = 0.0;
              scroll.sx     = 0.0;
              scroll.sy     = 0.0;
              scroll.active = 0;
              (*update)(ev);
            }
          }

          ev.type = EV_TIME;

          (*update)(ev);
          (*render)();
          SDL_GL_SwapWindow(window);
        }

        (*destroy)();

        SDL_StopTextInput();
        SDL_GL_DeleteContext(context);
      }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
  }
}

#endif
