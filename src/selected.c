/*
  stores songs selected in songlist
  does batch edit/conversion on songs
 */

#ifndef selected_h
#define selected_h

void selected_attach();
void selected_add(int i);
void selected_rem(int i);
void selected_rng(int i);
void selected_res();
int  selected_has(int i);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <stdio.h>

typedef struct _sel_rng_t
{
  int s;
  int e;
} sel_rng_t;

struct _selected_t
{
  sel_rng_t ranges[100];
  int       length;
} sel = {0};

void selected_add(int i)
{
  // check if song is in a range
  for (int index = 0; index < sel.length; index++)
  {
    sel_rng_t rng = sel.ranges[index];
    if (rng.s <= i && rng.e > i) return;
  }
  // store in a range
  sel.ranges[sel.length].s = i;
  sel.ranges[sel.length].e = i + 1;
  sel.length++;
}

void selected_rem(int i)
{
  // find songs range
  for (int index = 0; index < sel.length; index++)
  {
    sel_rng_t rng = sel.ranges[index];
    if (rng.s <= i && rng.e > i)
    {
      if (rng.s == i)
      {
        if (rng.e == i + 1)
        {
          // move indexes to invalid area
          sel.ranges[index].s = -1;
          sel.ranges[index].e = -1;
        }
        else
        {
          // exclude item
          sel.ranges[index].s += 1;
        }
      }
      else
      {
        // add a new range
        sel.ranges[index].e      = i;
        sel.ranges[sel.length].s = i + 1;
        sel.ranges[sel.length].e = rng.e;
        sel.length++;
      }
    }
  }
}

void selected_rng(int i)
{
  // extend last range
  if (i > sel.ranges[sel.length - 1].s)
    sel.ranges[sel.length - 1].e = i + 1;
  else
    sel.ranges[sel.length - 1].s = i;
}

void selected_res()
{
  sel.length = 0;
}

int selected_has(int i)
{
  // check if song is in a range
  for (int index = 0; index < sel.length; index++)
  {
    sel_rng_t rng = sel.ranges[index];
    if (rng.s <= i && rng.e > i) return 1;
  }
  return 0;
}

#endif
