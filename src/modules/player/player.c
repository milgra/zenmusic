/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef player_h
#define player_h

void player_play(char* path);
void player_stop();
void player_draw();
void player_get_album(const char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "libavformat/avformat.h"
#include "stream.c"

static AVInputFormat* file_iformat;

VideoState* is             = NULL;
double      remaining_time = 0.0;

void player_draw_video(int text_unit_both, int tex_unit_left, int tex_unit_right, int width, int height)
{
}

void player_draw_spectrum(int text_unit_both, int tex_unit_left, int tex_unit_right, int width, int height)
{
}

void player_play(char* path)
{
  if (is == NULL)
  {
    printf("player play %s\n", path);

    player_get_album(path);

    is = stream_open(path, file_iformat);

    printf("videostate %xu\n", is);
  }
}

void player_stop()
{
  if (is != NULL)
  {
    printf("player stop\n");
    stream_close(is);
    is = NULL;
  }
}

void player_draw()
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
      video_refresh(is, &remaining_time);
  }
}

void player_get_album(const char* path)
{
  int i, ret = 0;

  if (!path)
  {
    printf("Path is NULL\n");
    return;
  }

  AVFormatContext* pFormatCtx = avformat_alloc_context();

  printf("Opening %s\n", path);

  // open the specified path
  if (avformat_open_input(&pFormatCtx, path, NULL, NULL) != 0)
  {
    printf("avformat_open_input() failed");
    goto fail;
  }

  // read the format headers
  if (pFormatCtx->iformat->read_header(pFormatCtx) < 0)
  {
    printf("could not read the format header\n");
    goto fail;
  }

  // find the first attached picture, if available
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)
    {
      AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
      printf("ALBUM ART SIZE %i\n", pkt.size);
      //ret = fwrite(pkt.data, pkt.size, 1, album_art);
      av_free_packet(&pkt);
      break;
    }

fail:
  av_free(pFormatCtx);
  // this line crashes for some reason...
  //avformat_free_context(pFormatCtx);
}

#endif
