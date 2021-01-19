/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef player_h
#define player_h

#include "mtbitmap.c"
#include "mtmap.c"

void player_play(char* path);

int  player_toggle_pause();
void player_toggle_mute();

void player_set_volume(float ratio);
void player_set_position(float ratio);

double player_time();
double player_volume();
double player_duration();

void player_draw_video(bm_t* bm, int edge);
void player_draw_video_to_texture(int index, int w, int h);
void player_draw_waves(int channel, bm_t* bm, int edge);
void player_draw_rdft(int index, int channel, bm_t* bm);
void player_refresh();

bm_t* player_get_album(const char* path);
void  player_get_metadata(const char* path, map_t* map);
int   player_set_metadata(map_t* data, char* img_path);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL_image.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "mtcstring.c"
#include "mtgraphics.c"
#include "mtlog.c"
#include "render.c"
#include "strcomm.c"
#include "stream.c"
#include <assert.h>

static AVInputFormat* file_iformat;

VideoState* is             = NULL;
double      remaining_time = 0.0;

void player_play(char* path)
{
  if (is != NULL) stream_close(is);

  printf("LOG : playing %s\n", path);

  is = stream_open(path, file_iformat);
}

int player_toggle_pause()
{
  if (is)
  {
    stream_toggle_pause(is);
    is->step = 0;

    return is->paused;
  }

  return 0;
}

void player_toggle_mute()
{
  if (is)
  {
    is->muted = !is->muted;
  }
}

double player_time()
{
  if (is != NULL)
  {
    return get_master_clock(is);
  }
  else
    return 0.0;
}

double player_duration()
{
  if (is != NULL)
  {
    return is->duration;
  }
  else
    return 0.0;
}

void player_set_position(float ratio)
{
  if (is)
  {
    printf("ratio %f\n", ratio);
    printf("duration %f\n", player_duration());
    int newpos = (int)player_duration() * ratio;
    int diff   = (int)player_time() - newpos;
    printf("newpos %i\n", newpos);
    stream_seek(is, (int64_t)(newpos * AV_TIME_BASE), (int64_t)(diff * AV_TIME_BASE), 0);
  }
}

double player_volume()
{
  if (is != NULL)
  {
    return (float)is->audio_volume / (float)SDL_MIX_MAXVOLUME;
  }
  else
    return 1.0;
}

void player_set_volume(float ratio)
{
  /* double volume_level = is->audio_volume ? (20 * log(is->audio_volume / (double)SDL_MIX_MAXVOLUME) / log(10)) : -1000.0; */
  /* int    new_volume   = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0)); */
  /* is->audio_volume    = av_clip(is->audio_volume == new_volume ? (is->audio_volume + sign) : new_volume, 0, SDL_MIX_MAXVOLUME); */
  is->audio_volume = (int)((float)SDL_MIX_MAXVOLUME * ratio);
}

void player_refresh()
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      video_refresh(is, &remaining_time, 0);
    }
  }
}

void player_draw_video_to_texture(int index, int w, int h)
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      video_show(is, index, w, h, NULL, 0);
    }
  }
}

void player_draw_video(bm_t* bm, int edge)
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      video_show(is, 0, bm->w - 2 * edge, bm->h - 2 * edge, bm, 3);
    }
  }
}

void player_draw_waves(int channel, bm_t* bm, int edge)
{
  if (is != NULL)
  {
    if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
    {
      render_draw_waves(is, channel, bm, edge);
    }
  }
}

void player_draw_rdft(int index, int channel, bm_t* bm)
{
}

void player_get_metadata(const char* path, map_t* map)
{
  assert(path != NULL);

  int i, ret = 0;

  AVFormatContext* pFormatCtx  = avformat_alloc_context();
  AVDictionary*    format_opts = NULL;
  AVInputFormat*   fileformat  = NULL;

  av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);

  /* // open the specified path */
  if (avformat_open_input(&pFormatCtx, path, fileformat, &format_opts) != 0)
  {
    printf("avformat_open_input() failed");
  }

  av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

  AVDictionaryEntry* tag = NULL;

  while ((tag = av_dict_get(pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
  {
    char* value = cstr_fromcstring(tag->value);
    MPUT(map, tag->key, value);
    REL(value);
  }
}

int player_set_metadata(map_t* data, char* img_path)
{
  char* path = MGET(data, "path");

  if (path)
  {
    LOG("player_set_metadata for %s\n", path);

    // open cover art first

    int res;

    AVPacket*          img_pkt      = NULL;
    AVFormatContext*   img_ctx      = NULL;
    AVCodecParameters* img_codecpar = NULL;

    if (img_path)
    {
      LOG("player_set_metadata opening image file %s\n", img_path);

      img_ctx = avformat_alloc_context();
      res     = avformat_open_input(&img_ctx, img_path, 0, 0);

      if (res >= 0)
      {
        res = avformat_find_stream_info(img_ctx, 0);

        if (res >= 0)
        {

          img_pkt       = av_packet_alloc();
          img_pkt->data = NULL;
          img_pkt->size = 0;
          av_init_packet(img_pkt);

          while (av_read_frame(img_ctx, img_pkt) == 0)
          {
            if (img_ctx->streams[img_pkt->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
              img_codecpar = img_ctx->streams[img_pkt->stream_index]->codecpar;
              break;
            }
          }
        }
        else
          LOG("ERROR player_set_metadata cannot find stream info : %s\n", img_path);
      }
      else
        LOG("ERROR player_set_metadata cannot open image file : %s\n", img_path);
    }

    // open source file

    AVFormatContext* src_ctx = avformat_alloc_context();

    res = avformat_open_input(&src_ctx, path, 0, 0);

    if (res >= 0)
    {
      res = avformat_find_stream_info(src_ctx, 0);

      if (res >= 0)
      {
        AVFormatContext* out_ctx;
        AVOutputFormat*  out_fmt = av_guess_format("mp3", "./result.mp3", NULL);

        res = avformat_alloc_output_context2(&out_ctx, out_fmt, "mp3", "./result.mp3");

        if (res >= 0)
        {
          // creating streams present in input file except cover art

          for (unsigned i = 0; i < src_ctx->nb_streams; i++)
          {
            if ((src_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) == 0)
            {
              const AVCodec* codec = avcodec_find_encoder(src_ctx->streams[i]->codecpar->codec_id);
              if (codec)
              {
                AVStream* ostream = avformat_new_stream(out_ctx, codec);
                avcodec_parameters_copy(ostream->codecpar, src_ctx->streams[i]->codecpar);
                ostream->codecpar->codec_tag = 0;
              }
            }
          }

          // create cover art image stream

          if (img_codecpar)
          {
            AVCodec* codec = avcodec_find_encoder(img_codecpar->codec_id);
            if (codec)
            {
              AVStream* ostream = avformat_new_stream(out_ctx, codec);
              avcodec_parameters_copy(ostream->codecpar, img_codecpar);
              ostream->codecpar->codec_tag = 0;
              ostream->disposition |= AV_DISPOSITION_ATTACHED_PIC;
            }
          }

          // copy metadata in old file to new file

          av_dict_copy(&out_ctx->metadata, src_ctx->metadata, 0);

          // update with new entries from data map

          av_dict_set(&out_ctx->metadata, "title", "testtest", 0);

          if (!(out_ctx->oformat->flags & AVFMT_NOFILE))
          {
            avio_open(&out_ctx->pb, "./result.mp3", AVIO_FLAG_WRITE);

            res = avformat_init_output(out_ctx, NULL);

            if (res >= 0)
            {

              res = avformat_write_header(out_ctx, NULL);

              if (res >= 0)
              {
                AVPacket* src_pkt = av_packet_alloc();
                av_init_packet(src_pkt);
                src_pkt->data = NULL;
                src_pkt->size = 0;

                // copy all packets from old file to new file with the exception of cover art image

                while (av_read_frame(src_ctx, src_pkt) == 0)
                {
                  if ((src_ctx->streams[src_pkt->stream_index]->disposition & AV_DISPOSITION_ATTACHED_PIC) == 0)
                  {
                    src_pkt->stream_index = 0;
                    av_write_frame(out_ctx, src_pkt);
                  }
                }

                // if no cover art is added during saving, add a new stream

                if (img_codecpar)
                {
                  img_pkt->stream_index = 1;

                  res = av_write_frame(out_ctx, img_pkt);
                  if (res < 0)
                    LOG("ERROR : player_set_metadata : cannot write cover art image packet\n");

                  // cleanup
                  av_packet_free(&img_pkt);
                  avformat_close_input(&img_ctx);
                  avformat_free_context(img_ctx);
                }

                av_packet_free(&src_pkt);
                av_write_trailer(out_ctx);

                avformat_close_input(&src_ctx);
                avformat_free_context(out_ctx);
                avformat_free_context(src_ctx);
              }
              else
                LOG("ERROR : player_set_metadata : cannot write header\n");
            }
            else
              LOG("ERROR : player_set_metadata : cannot init output\n");
          }
          else
            LOG("ERROR : player_set_metadata : avformat needs no file\n");
        }
        else
          LOG("ERROR : player_set_metadata : cannot allocate output context\n");
      }
      else
        LOG("ERROR : player_set_metadata : cannot find stream info\n");
    }
    else
      LOG("ERROR : player_set_metadata : cannot open input file\n");
  }
  else
    LOG("ERROR : player_set_metadata : no path present in entry map\n");

  return 0;
}

bm_t* player_get_album(const char* path)
{
  assert(path != NULL);

  int i, ret = 0;

  AVFormatContext* pFormatCtx = avformat_alloc_context();

  /* // open the specified path */
  if (avformat_open_input(&pFormatCtx, path, NULL, NULL) != 0)
  {
    printf("avformat_open_input() failed");
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
      gfx_insert_rgb(result, image->pixels, image->w, image->h, 0, 0);
      //memcpy(result->data, image->pixels, image->w * image->h * 4);

      av_packet_unref(&pkt);
      break;
    }

  //avformat_free_context(pFormatCtx);

  printf("returning\n");
  return result;

fail:
  av_free(pFormatCtx);
  return NULL;
  // this line crashes for some reason...
  //avformat_free_context(pFormatCtx);
}

// get duration

/* void openVideoFile(char *filename) { */
/*     AVFormatContext* pFormatCtx; */
/*     AVCodecContext* pCodecCtx; */
/*     int videoStream = -1; */
/*     int i = 0; */

/* 	// open video file */
/*     int ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL); */
/*     if (ret != 0) { */
/*         printf("Unable to open video file: %s\n", filename); */
/*         return; */
/*     } */

/*     // Retrieve stream information */
/*     ret = avformat_find_stream_info(pFormatCtx, NULL); */
/*     assert(ret >= 0); */

/*     printf("\n"); */
/*     printf("Duration: %lus\n", pFormatCtx->duration/1000000); */
/* } */

// get width and height

/* void openVideoFile(char *filename) { */
/* 	AVFormatContext* pFormatCtx; */
/* 	AVCodecContext* pCodecCtx; */
/* 	int videoStream = -1; */
/*     int i = 0; */

/* 	// open video file */
/*     int ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL); */
/*     if (ret != 0) { */
/*         printf("Unable to open video file: %s\n", filename); */
/*         return; */
/*     } */

/*     // Retrieve stream information */
/*     ret = avformat_find_stream_info(pFormatCtx, NULL); */
/*     assert(ret >= 0); */

/*     for(i = 0; i < pFormatCtx->nb_streams; i++) { */
/*         if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO && videoStream < 0) { */
/*             videoStream = i;             */
/*         } */
/*     } // end for i */
/*     assert(videoStream != -1); */

/*     // Get a pointer to the codec context for the video stream */
/*     pCodecCtx=pFormatCtx->streams[videoStream]->codec; */
/*     assert(pCodecCtx != NULL); */

/*     printf("\n"); */
/*     printf("Width: %d\n", pCodecCtx->width); */
/*     printf("Height: %d\n", pCodecCtx->height); */
/* } */

#endif
