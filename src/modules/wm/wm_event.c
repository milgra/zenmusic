#ifndef event_h
#define event_h

#include <SDL.h>
#include <stdint.h>

enum evtype
{
  EV_EMPTY,
  EV_TIME,
  EV_RESIZE,
  EV_MMOVE,
  EV_MDOWN,
  EV_MDOWN_OUTSIDE,
  EV_MUP,
  EV_SCROLL,
  EV_KDOWN,
  EV_KUP,
  EV_TEXT,
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
  int         button;
  SDL_KeyCode keycode;
} ev_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#endif
