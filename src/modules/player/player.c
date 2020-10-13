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

  /* file_iformat = av_find_input_format(); */
  /* if (!file_iformat) */
  /* { */
  /*   av_log(NULL, AV_LOG_FATAL, "Unknown input format: %s\n", arg); */
  /*   return AVERROR(EINVAL); */
  /* } */

  is = stream_open("akarmi.mp3", file_iformat);
  if (!is)
  {
    av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
  }
}

#endif
