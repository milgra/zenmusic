/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef player_h
#define player_h

void player_init();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "libavformat/avformat.h"
#include "stream.c"

static AVInputFormat* file_iformat;

void player_init()
{
  VideoState* is;

  is = stream_open("res/marja.mp4", file_iformat);

  printf("videostate %xu\n", is);

  if (!is)
  {
    av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
  }
}

#endif
