/*
  stores songs selected in songlist
  does batch edit/conversion on songs
 */

#ifndef selected_h
#define selected_h

#include "mtmap.c"
#include "view.c"

void selected_attach();
void selected_add(map_t* song);
void selected_rem(map_t* song);

#endif

#if __INCLUDE_LEVEL__ == 0

void selected_attach()
{
}

void selected_add(map_t* song)
{
}

void selected_rem(map_t* song)
{
}

#endif
