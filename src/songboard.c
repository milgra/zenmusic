/*
  stores songs selected in songlist
  does batch edit/conversion on songs
 */

#ifndef songboard_h
#define songboard_h

#include "mtmap.c"
#include "view.c"

void songboard_attach();
void songboard_add(map_t* song);
void songboard_rem(map_t* song);

#endif

#if __INCLUDE_LEVEL__ == 0

void songboard_attach()
{
}

void songboard_add(map_t* song)
{
}

void songboard_rem(map_t* song)
{
}

#endif
