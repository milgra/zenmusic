/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef player_h
#define player_h

void player_init();
void player_draw();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "libavformat/avformat.h"
#include "stream.c"

static AVInputFormat* file_iformat;

VideoState* is;
double      remaining_time = 0.0;

void player_init()
{

  is = stream_open("res/marja.mp4", file_iformat);

  printf("videostate %xu\n", is);

  if (!is)
  {
    av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
  }
}

void player_draw()
{
  if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    video_refresh(is, &remaining_time);

  printf("draw, remaining time %f\n", remaining_time);
}

#endif
