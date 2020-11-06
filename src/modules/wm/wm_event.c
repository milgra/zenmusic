#ifndef event_h
#define event_h

#include <SDL.h>
#include <stdint.h>

enum evtype
{
  EV_EMPTY,
  EV_MDOWN,
  EV_MUP,
  EV_MMOVE,
  EV_KDOWN,
  EV_KUP,
  EV_SCROLL,
  EV_TEXT,
  EV_RESIZE,
  EV_TIME
};

typedef struct _ev_t
{
  enum evtype type;
  int         x;
  int         y;
  float       dx;
  float       dy;
  int         w;
  int         h;
  char*       text;
  char        drag;
  uint32_t    time;
  uint32_t    dtime;
  SDL_KeyCode keycode;
} ev_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#endif
