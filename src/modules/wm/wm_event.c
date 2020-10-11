#ifndef event_h
#define event_h

#include <stdint.h>

enum evtype
{
  EV_EMPTY,
  EV_MDOWN,
  EV_MUP,
  EV_MMOVE,
  EV_TEXT,
  EV_RESIZE
};

typedef struct _ev_t
{
  enum evtype type;
  int         x;
  int         y;
  int         dx;
  int         dy;
  int         w;
  int         h;
  char*       text;
  char        drag;
  uint32_t    time;
} ev_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#endif
