/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef editor_h
#define editor_h

#include "mtbitmap.c"
#include "mtmap.c"
#include "mtvector.c"

void editor_update_metadata(vec_t* songs, map_t* data);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL_image.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "mtcstring.c"
#include "mtgraphics.c"
#include "mtlog.c"

int editor_get_metadata(const char* path, map_t* map)
{
  assert(path != NULL);
  assert(map != NULL);

  int retv = -1;

  AVFormatContext* pFormatCtx  = avformat_alloc_context();
  AVDictionary*    format_opts = NULL;

  av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);

  /* // open the specified path */
  if (avformat_open_input(&pFormatCtx, path, NULL, &format_opts) == 0)
  {
    if (pFormatCtx)
    {
      retv = 0;

      av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

      AVDictionaryEntry* tag = NULL;

      while ((tag = av_dict_get(pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
      {
        char* value = cstr_fromcstring(tag->value);
        MPUT(map, tag->key, value);
        REL(value);
      }

      // Retrieve stream information
      retv = avformat_find_stream_info(pFormatCtx, NULL);

      if (retv >= 0)
      {
        int   dur   = pFormatCtx->duration / 1000000;
        char* dur_s = mem_calloc(10, "char*", NULL, NULL);
        snprintf(dur_s, 10, "%i:%.2i", (int)dur / 60, dur - (int)(dur / 60) * 60);
        MPUT(map, "duration", dur_s);
        REL(dur_s);
      }
      else
      {
        printf("editor_get_metadata no stream information found!!!\n");
        MPUT(map, "duration", cstr_fromcstring("0"));
      }
    }
    else
      printf("editor_get_metadata no media context for %s\n", path);

    avformat_close_input(&pFormatCtx);
  }
  else
    printf("editor_get_metadata cannot open input %s\n", path);

  return retv;
}

int editor_set_metadata(map_t* data, char* img_path)
{
  char* path = MGET(data, "path");

  if (path)
  {
    LOG("editor_set_metadata for %s\n", path);

    // open cover art first

    int res;

    AVPacket*          img_pkt      = NULL;
    AVFormatContext*   img_ctx      = NULL;
    AVCodecParameters* img_codecpar = NULL;

    if (img_path)
    {
      LOG("editor_set_metadata opening image file %s\n", img_path);

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
          LOG("ERROR editor_set_metadata cannot find stream info : %s\n", img_path);
      }
      else
        LOG("ERROR editor_set_metadata cannot open image file : %s\n", img_path);
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
                    LOG("ERROR : editor_set_metadata : cannot write cover art image packet\n");

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
                LOG("ERROR : editor_set_metadata : cannot write header\n");
            }
            else
              LOG("ERROR : editor_set_metadata : cannot init output\n");
          }
          else
            LOG("ERROR : editor_set_metadata : avformat needs no file\n");
        }
        else
          LOG("ERROR : editor_set_metadata : cannot allocate output context\n");
      }
      else
        LOG("ERROR : editor_set_metadata : cannot find stream info\n");
    }
    else
      LOG("ERROR : editor_set_metadata : cannot open input file\n");
  }
  else
    LOG("ERROR : editor_set_metadata : no path present in entry map\n");

  return 0;
}

bm_t* editor_get_album(const char* path)
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
