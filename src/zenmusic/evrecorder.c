#ifndef evrec_h
#define evrec_h

#include "wm_event.c"

void  evrec_init_recorder(char* path);
void  evrec_init_player(char* path);
void  evrec_close();
void  evrec_record(ev_t event);
ev_t* evrec_replay(uint32_t time);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_vector.c"
#include <stdio.h>

struct evrec_t
{
  FILE*  file;
  vec_t* events;
  int    index;
} rec = {0};

void evrec_init_recorder(char* path)
{
  FILE* file = fopen(path, "w");
  if (!file) printf("evrec : cannot open file %s\n", path);
  rec.file = file;
}

void evrec_init_player(char* path)
{
  FILE* file = fopen(path, "r");
  if (!file) printf("evrec : cannot open file %s\n", path);

  rec.file   = file;
  rec.events = VNEW();

  while (1)
  {
    char type[100];
    ev_t ev    = {0};
    int  count = fscanf(file, "%u %s", &ev.time, type);

    if (count == 2)
    {
      if (strcmp(type, "mmove") == 0) fscanf(file, " %i %i %f %f %c\n", &ev.x, &ev.y, &ev.dx, &ev.dy, &ev.drag);
      if (strcmp(type, "mdown") == 0) fscanf(file, " %i %i %i %i %i %i\n", &ev.x, &ev.y, &ev.button, &ev.dclick, &ev.ctrl_down, &ev.shift_down);
      if (strcmp(type, "mup") == 0) fscanf(file, " %i %i %i %i %i %i\n", &ev.x, &ev.y, &ev.button, &ev.dclick, &ev.ctrl_down, &ev.shift_down);
      if (strcmp(type, "scroll") == 0) fscanf(file, " %f %f\n", &ev.dx, &ev.dy);
      if (strcmp(type, "kdown") == 0) fscanf(file, " %i\n", &ev.keycode);
      if (strcmp(type, "kup") == 0) fscanf(file, " %i\n", &ev.keycode);
      if (strcmp(type, "text") == 0) fscanf(file, " %s\n", ev.text);
      if (strcmp(type, "resize") == 0) fscanf(file, " %i %i\n", &ev.w, &ev.h);

      if (strcmp(type, "mmove") == 0) ev.type = EV_MMOVE;
      if (strcmp(type, "mdown") == 0) ev.type = EV_MDOWN;
      if (strcmp(type, "mup") == 0) ev.type = EV_MUP;
      if (strcmp(type, "scroll") == 0) ev.type = EV_SCROLL;
      if (strcmp(type, "kdown") == 0) ev.type = EV_KDOWN;
      if (strcmp(type, "kup") == 0) ev.type = EV_KUP;
      if (strcmp(type, "text") == 0) ev.type = EV_TEXT;
      if (strcmp(type, "resize") == 0) ev.type = EV_RESIZE;
    }
    else
      break;

    VADDR(rec.events, HEAP(ev, "ev_t"));
  }

  fclose(file);

  printf("%i events read\n", rec.events->length);
}

void evrec_close()
{
  fclose(rec.file);
}

void evrec_record(ev_t ev)
{
  if (ev.type == EV_MMOVE) fprintf(rec.file, "%u mmove %i %i %f %f %i\n", ev.time, ev.x, ev.y, ev.dx, ev.dy, ev.drag);
  if (ev.type == EV_MDOWN) fprintf(rec.file, "%u mdown %i %i %i %i %i %i\n", ev.time, ev.x, ev.y, ev.button, ev.dclick, ev.ctrl_down, ev.shift_down);
  if (ev.type == EV_MUP) fprintf(rec.file, "%u mup %i %i %i %i %i %i\n", ev.time, ev.x, ev.y, ev.button, ev.dclick, ev.ctrl_down, ev.shift_down);
  if (ev.type == EV_SCROLL) fprintf(rec.file, "%u scroll %f %f\n", ev.time, ev.dx, ev.dy);
  if (ev.type == EV_KDOWN) fprintf(rec.file, "%u kdown %i\n", ev.time, ev.keycode);
  if (ev.type == EV_KUP) fprintf(rec.file, "%u kup %i\n", ev.time, ev.keycode);
  if (ev.type == EV_TEXT) fprintf(rec.file, "%u text %s\n", ev.time, ev.text);
  if (ev.type == EV_RESIZE) fprintf(rec.file, "%u resize %i %i\n", ev.time, ev.w, ev.h);
}

ev_t* evrec_replay(uint32_t time)
{
  ev_t* event = rec.events->data[rec.index];

  // printf("time %u event time %u event type %i\n", time, event->time, event->type);

  if (event->time < time)
  {
    rec.index++;
    return event;
  }

  return NULL;
}

#endif
