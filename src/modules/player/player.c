/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef player_h
#define player_h

#include "mtbitmap.c"

void  player_play(char* path);
void  player_stop();
void  player_draw();
bm_t* player_get_album(const char* path);
void  player_draw_video(int index, int w, int h);
void  player_draw_waves(int index, int channel, bm_t* bm);
void  player_draw_rdft(int index, int channel, bm_t* bm);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL_image.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "render.c"
#include "stream.c"

static AVInputFormat* file_iformat;

VideoState* is             = NULL;
double      remaining_time = 0.0;

void player_play(char* path)
{
  if (is == NULL)
  {
    printf("player play %s\n", path);

    is = stream_open(path, file_iformat);
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

void player_draw_video(int index, int w, int h)
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      video_refresh(is, &remaining_time, index);
      video_show(is, index);
    }
  }
}

void player_draw_waves(int index, int channel, bm_t* bm)
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      render_draw_waves(is, channel, bm);
    }
  }
}

void player_draw_rdft(int index, int channel, bm_t* bm)
{
}

bm_t* player_get_album(const char* path)
{
  int i, ret = 0;

  if (!path)
  {
    printf("Path is NULL\n");
    return NULL;
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

  bm_t* result = NULL;

  // find the first attached picture, if available
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)
    {
      AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
      printf("ALBUM ART SIZE %i\n", pkt.size);

      SDL_RWops*   rw = SDL_RWFromMem(pkt.data, pkt.size);
      SDL_Surface* image;
      image = IMG_Load_RW(rw, 1);
      if (!image)
      {
        printf("IMG_Load_RW: %s\n", IMG_GetError());
        // handle error
      }
      else
        printf("IMG SUCCESS %i %i %i\n", image->format->BytesPerPixel, image->w, image->h);

      /* int            components, w, h; */
      /* unsigned char* bytes = stbi_load_from_memory(pkt.data, pkt.size, &w, &h, &components, 4); */

      result = bm_new(image->w, image->h);
      bm_from3(result, image->pixels);
      //memcpy(result->data, image->pixels, image->w * image->h * 4);

      av_packet_unref(&pkt);
      break;
    }

  printf("returning\n");
  return result;

fail:
  av_free(pFormatCtx);
  return NULL;
  // this line crashes for some reason...
  //avformat_free_context(pFormatCtx);
}

#endif
