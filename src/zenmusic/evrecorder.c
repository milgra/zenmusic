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
  if (!file) printf("evrec recorder : cannot open file %s\n", path);
  rec.file = file;
}

void evrec_init_player(char* path)
{
  FILE* file = fopen(path, "r");
  if (!file) printf("evrec player : cannot open file %s\n", path);

  rec.file   = file;
  rec.events = VNEW();

  char line[1000] = {0};
  char type[100]  = {0};
  char done       = 1;
  ev_t ev         = {0};

  while (1)
  {

    if (fgets(line, 1000, file) != NULL)
    {
      if (done)
      {
        sscanf(line, "%u %s", &ev.time, type);

        done = 0;
      }
      else
      {

        if (strcmp(type, "mmove") == 0) sscanf(line, "%i %i %f %f %c\n", &ev.x, &ev.y, &ev.dx, &ev.dy, &ev.drag);
        if (strcmp(type, "mdown") == 0) sscanf(line, "%i %i %i %i %i %i\n", &ev.x, &ev.y, &ev.button, &ev.dclick, &ev.ctrl_down, &ev.shift_down);
        if (strcmp(type, "mup") == 0) sscanf(line, "%i %i %i %i %i %i\n", &ev.x, &ev.y, &ev.button, &ev.dclick, &ev.ctrl_down, &ev.shift_down);
        if (strcmp(type, "scroll") == 0) sscanf(line, "%f %f\n", &ev.dx, &ev.dy);
        if (strcmp(type, "kdown") == 0) sscanf(line, "%i\n", &ev.keycode);
        if (strcmp(type, "kup") == 0) sscanf(line, "%i\n", &ev.keycode);
        if (strcmp(type, "text") == 0) memcpy(ev.text, line, strlen(line) - 1);
        if (strcmp(type, "resize") == 0) sscanf(line, "%i %i\n", &ev.w, &ev.h);

        if (strcmp(type, "mmove") == 0) ev.type = EV_MMOVE;
        if (strcmp(type, "mdown") == 0) ev.type = EV_MDOWN;
        if (strcmp(type, "mup") == 0) ev.type = EV_MUP;
        if (strcmp(type, "scroll") == 0) ev.type = EV_SCROLL;
        if (strcmp(type, "kdown") == 0) ev.type = EV_KDOWN;
        if (strcmp(type, "kup") == 0) ev.type = EV_KUP;
        if (strcmp(type, "text") == 0) ev.type = EV_TEXT;
        if (strcmp(type, "resize") == 0) ev.type = EV_RESIZE;

        VADDR(rec.events, HEAP(ev, "ev_t"));

        done = 1;
      }
    }
    else
      break;
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
  if (ev.type == EV_MMOVE) fprintf(rec.file, "%u mmove\n%i %i %f %f %i\n", ev.time, ev.x, ev.y, ev.dx, ev.dy, ev.drag);
  if (ev.type == EV_MDOWN) fprintf(rec.file, "%u mdown\n%i %i %i %i %i %i\n", ev.time, ev.x, ev.y, ev.button, ev.dclick, ev.ctrl_down, ev.shift_down);
  if (ev.type == EV_MUP) fprintf(rec.file, "%u mup\n%i %i %i %i %i %i\n", ev.time, ev.x, ev.y, ev.button, ev.dclick, ev.ctrl_down, ev.shift_down);
  if (ev.type == EV_SCROLL) fprintf(rec.file, "%u scroll\n%f %f\n", ev.time, ev.dx, ev.dy);
  if (ev.type == EV_KDOWN) fprintf(rec.file, "%u kdown\n%i\n", ev.time, ev.keycode);
  if (ev.type == EV_KUP) fprintf(rec.file, "%u kup\n%i\n", ev.time, ev.keycode);
  if (ev.type == EV_TEXT) fprintf(rec.file, "%u text\n%s\n", ev.time, ev.text);
  if (ev.type == EV_RESIZE) fprintf(rec.file, "%u resize\n%i %i\n", ev.time, ev.w, ev.h);
}

ev_t* evrec_replay(uint32_t time)
{
  if (rec.index < rec.events->length)
  {
    ev_t* event = rec.events->data[rec.index];

    // printf("time %u event time %u event type %i\n", time, event->time, event->type);

    if (event->time < time)
    {
      rec.index++;
      return event;
    }
  }

  return NULL;
}

#endif
