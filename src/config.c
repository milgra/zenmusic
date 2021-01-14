#ifndef config_h
#define config_h

void config_init();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"

void config_init()
{
}

#endif
