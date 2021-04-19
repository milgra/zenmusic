/*
  This module is based on ffplay.c so all bugfixes/updates to ffplay should be introduced to this module.
 */

#ifndef editor_h
#define editor_h

#include "mtbitmap.c"
#include "mtmap.c"
#include "mtvector.c"

void  editor_update_metadata(char* libpath, vec_t* songs, map_t* data, vec_t* drop, char* cover);
int   editor_get_metadata(const char* path, map_t* map);
int   editor_set_metadata(map_t* data, char* img_path);
void  editor_get_album(const char* path, bm_t* bitmap);
void  editor_update_song_metadata(char* libpath, char* path, map_t* data, vec_t* drop, char* cover);
bm_t* editor_get_image(const char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "mtcstring.c"
#include "mtgraphics.c"
#include "mtlog.c"
#include "mtmemory.c"

void editor_update_metadata(char* libpath, vec_t* songs, map_t* data, vec_t* drop, char* cover)
{
  /* printf("editor update metadata for songs:\n"); */
  /* mem_describe(songs, 0); */
  /* printf("\ndata:\n"); */
  /* mem_describe(data, 0); */
  /* printf("\ndrop:\n"); */
  /* mem_describe(drop, 0); */
  /* printf("\ncover %s\n", cover); */

  for (int index = 0; index < songs->length; index++)
  {
    map_t* song = songs->data[index];
    char*  path = MGET(song, "file/path");
    editor_update_song_metadata(libpath, path, data, drop, cover);
  }
}

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
        char* key   = cstr_fromformat("%s/%s", "meta", tag->key, NULL);
        MPUT(map, key, value);
        REL(value);
      }

      // Retrieve stream information
      retv = avformat_find_stream_info(pFormatCtx, NULL);

      if (retv >= 0)
      {
        int   dur   = pFormatCtx->duration / 1000000;
        char* dur_s = mem_calloc(10, "char*", NULL, NULL);
        snprintf(dur_s, 10, "%i:%.2i", (int)dur / 60, dur - (int)(dur / 60) * 60);
        MPUT(map, "file/duration", dur_s);
        REL(dur_s);
      }
      else
      {
        printf("editor_get_metadata no stream information found!!!\n");
        MPUT(map, "file/duration", cstr_fromcstring("0"));
      }

      for (unsigned i = 0; i < pFormatCtx->nb_streams; i++)
      {
        AVCodecParameters* param = pFormatCtx->streams[i]->codecpar;
        const AVCodec*     codec = avcodec_find_encoder(pFormatCtx->streams[i]->codecpar->codec_id);
        if (codec)
        {
          if (param->codec_type == AVMEDIA_TYPE_AUDIO)
          {
            char* channels   = mem_calloc(10, "char*", NULL, NULL);
            char* bitrate    = mem_calloc(10, "char*", NULL, NULL);
            char* samplerate = mem_calloc(10, "char*", NULL, NULL);

            snprintf(channels, 10, "%i", param->channels);
            snprintf(bitrate, 10, "%li", param->bit_rate);
            snprintf(samplerate, 10, "%i", param->sample_rate);
            MPUT(map, "file/channels", channels);
            MPUT(map, "file/bit_rate", bitrate);
            MPUT(map, "file/sample_rate", samplerate);
          }
        }
      }
    }
    else
    {
      printf("editor_get_metadata no media context for %s\n", path);
    }

    avformat_close_input(&pFormatCtx);
  }
  else
  {
    printf("editor_get_metadata cannot open input %s\n", path);
  }

  return retv;
}

void editor_update_song_metadata(char* libpath, char* path, map_t* data, vec_t* drop, char* cover_image)
{
  LOG("editor_update_song_metadata for %s\n", path);

  // create temporary name

  char* tmp;
  char* dot = strrchr(path, '.');
  if (dot)
  {
    int   length = dot - path;
    char* name   = mem_calloc(length, "char*", NULL, NULL);
    memcpy(name, path, length);
    tmp = cstr_fromformat("%s_tmp%s", name, dot, NULL);
  }
  else
  {
    tmp = cstr_fromformat("%s_tmp", path, NULL);
  }

  char* oldpath = cstr_fromformat("%s%s", libpath, path, NULL);
  char* newpath = cstr_fromformat("%s%s", libpath, tmp, NULL);

  //printf("oldpath %s\n", oldpath);
  //printf("newpath %s\n", newpath);

  // open cover art first

  int res;

  AVPacket*          cover_packet  = NULL;
  AVFormatContext*   cover_context = NULL;
  AVCodecParameters* cover_cparam  = NULL;

  if (cover_image)
  {
    //LOG("editor_set_metadata opening image file %s\n", cover_image);

    cover_context = avformat_alloc_context();
    res           = avformat_open_input(&cover_context, cover_image, 0, 0);

    if (res >= 0)
    {
      res = avformat_find_stream_info(cover_context, 0);

      if (res >= 0)
      {

        cover_packet       = av_packet_alloc();
        cover_packet->data = NULL;
        cover_packet->size = 0;
        av_init_packet(cover_packet);

        while (av_read_frame(cover_context, cover_packet) == 0)
        {
          if (cover_context->streams[cover_packet->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
          {
            cover_cparam = cover_context->streams[cover_packet->stream_index]->codecpar;
            break;
          }
        }
      }
      else
        LOG("ERROR editor_set_metadata cannot find stream info : %s\n", cover_image);
    }
    else
      LOG("ERROR editor_set_metadata cannot open image file : %s\n", cover_image);
  }

  // open source file

  AVFormatContext* src_ctx = avformat_alloc_context();

  res = avformat_open_input(&src_ctx,
                            oldpath,
                            0,
                            0);

  if (res >= 0)
  {
    res = avformat_find_stream_info(src_ctx,
                                    0);

    if (res >= 0)
    {
      AVFormatContext* out_ctx;
      AVOutputFormat*  out_fmt = av_guess_format(NULL,
                                                tmp,
                                                NULL);

      res = avformat_alloc_output_context2(&out_ctx,
                                           out_fmt,
                                           NULL,
                                           newpath);

      if (res >= 0)
      {
        // creating streams present in input file except cover art

        for (unsigned i = 0; i < src_ctx->nb_streams; i++)
        {
          int dispos = src_ctx->streams[i]->disposition;
          // skip if we have new image for cover art and stream is cover art stream
          char skip = (dispos & AV_DISPOSITION_ATTACHED_PIC) == 1 && cover_cparam;

          if (!skip)
          {
            AVCodecParameters* param = src_ctx->streams[i]->codecpar;
            const AVCodec*     codec = avcodec_find_encoder(src_ctx->streams[i]->codecpar->codec_id);
            if (codec)
            {
              if (param->codec_type == AVMEDIA_TYPE_VIDEO)
              {
                //printf("Video Codec: resolution %d x %d\n", param->width, param->height);
              }
              else if (param->codec_type == AVMEDIA_TYPE_AUDIO)
              {
                //printf("Audio Codec: %d channels, sample rate %d\n", param->channels, param->sample_rate);
              }
              //printf("Codec %s ID %d bit_rate %ld\n", codec->long_name, codec->id, param->bit_rate);

              AVStream* ostream = avformat_new_stream(out_ctx,
                                                      codec);

              avcodec_parameters_copy(ostream->codecpar,
                                      src_ctx->streams[i]->codecpar);

              ostream->codecpar->codec_tag = 0;
            }
          }
        }

        // copy metadata in old file to new file

        av_dict_copy(&out_ctx->metadata, src_ctx->metadata, 0);

        AVDictionaryEntry* tag = NULL;

        // printf("existing tags:\n");
        // while ((tag = av_dict_get(out_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) printf("%s : %s\n", tag->key, tag->value);

        // update metadata in new file

        vec_t* fields = VNEW();
        map_keys(data, fields);

        for (int i = 0; i < fields->length; i++)
        {
          char* field = fields->data[i];
          char* value = MGET(data, field);

          printf("adding/updating %s to %s\n", field, value);

          av_dict_set(&out_ctx->metadata, field, value, 0);
        }

        // remove fields in new file

        for (int i = 0; i < drop->length; i++)
        {
          char* field = drop->data[i];

          printf("removing %s\n", field);

          av_dict_set(&out_ctx->metadata, field, NULL, 0);
        }

        if (!(out_ctx->oformat->flags & AVFMT_NOFILE))
        {

          avio_open(&out_ctx->pb,
                    newpath,
                    AVIO_FLAG_WRITE);

          res = avformat_init_output(out_ctx,
                                     NULL);

          if (res >= 0)
          {

            // create cover art image stream before writing header

            if (cover_cparam)
            {
              AVCodec* codec = avcodec_find_encoder(cover_cparam->codec_id);
              if (codec)
              {
                AVStream* ostream = avformat_new_stream(out_ctx,
                                                        codec);

                cover_packet->stream_index = ostream->index;

                printf("creating new stream for new cover, index : %i\n", ostream->index);
                avcodec_parameters_copy(ostream->codecpar,
                                        cover_cparam);

                ostream->codecpar->codec_tag = 0;
                ostream->disposition |= AV_DISPOSITION_ATTACHED_PIC;
              }
            }

            res = avformat_write_header(out_ctx,
                                        NULL);

            if (res >= 0)
            {
              AVPacket* src_pkt = av_packet_alloc();

              av_init_packet(src_pkt);
              src_pkt->data = NULL;
              src_pkt->size = 0;

              // copy all packets from old file to new file with the exception of cover art image

              while (av_read_frame(src_ctx, src_pkt) == 0)
              {

                int dispos = src_ctx->streams[src_pkt->stream_index]->disposition;
                // skip if we have new image for cover art and stream is cover art stream
                char skip = (dispos & AV_DISPOSITION_ATTACHED_PIC) == 1 && cover_cparam;

                if (!skip)
                {
                  //src_pkt->stream_index = 0;
                  av_write_frame(out_ctx, src_pkt);
                }
                else
                  printf("skippin cover stream\n");
              }

              // if no cover art is added during saving, add a new stream

              if (cover_cparam)
              {
                printf("adding new cover art\n");

                res = av_write_frame(out_ctx, cover_packet);

                if (res < 0) LOG("ERROR : editor_set_metadata : cannot write cover art image packet\n");

                // cleanup
                av_packet_free(&cover_packet);
                avformat_close_input(&cover_context);
                avformat_free_context(cover_context);
              }

              av_packet_free(&src_pkt);
              av_write_trailer(out_ctx);

              avformat_close_input(&src_ctx);
              avformat_free_context(out_ctx);
              avformat_free_context(src_ctx);

              // rename new file to old name

              if (rename(newpath, oldpath) != 0)
                LOG("ERROR : editor_set_metadata : cannot rename new file to old name\n");
              else
                printf("UPDATE SUCCESS\n");
            }
            else
              LOG("ERROR : editor_set_metadata : cannot write header\n");
          }
          else
            LOG("ERROR : editor_set_metadata : cannot init output\n");

          avio_close(out_ctx->pb);
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

void editor_get_album(const char* path, bm_t* bitmap)
{
  assert(path != NULL);

  printf("editor_get_album %s %i %i\n", path, bitmap->w, bitmap->h);

  int i, ret = 0;

  AVFormatContext* src_ctx = avformat_alloc_context();

  /* // open the specified path */
  if (avformat_open_input(&src_ctx, path, NULL, NULL) != 0)
  {
    printf("avformat_open_input() failed");
  }

  bm_t* result = NULL;

  // find the first attached picture, if available
  for (i = 0; i < src_ctx->nb_streams; i++)
  {
    if (src_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)
    {
      AVPacket pkt = src_ctx->streams[i]->attached_pic;

      printf("ALBUM ART SIZE %i\n", pkt.size);

      AVCodecParameters* param = src_ctx->streams[i]->codecpar;

      printf("Resolution %d x %d codec %i\n", param->width, param->height, param->codec_id);

      const AVCodec*  codec        = avcodec_find_decoder(param->codec_id);
      AVCodecContext* codecContext = avcodec_alloc_context3(codec);

      avcodec_parameters_to_context(codecContext, param);
      avcodec_open2(codecContext, codec, NULL);

      AVFrame* frame = av_frame_alloc();

      avcodec_send_packet(codecContext, &pkt);
      avcodec_receive_frame(codecContext, frame);

      printf("received frame %i %i\n", frame->width, frame->height);

      static unsigned sws_flags = SWS_BICUBIC;

      struct SwsContext* img_convert_ctx = sws_getContext(frame->width,
                                                          frame->height,
                                                          frame->format,
                                                          bitmap->w,
                                                          bitmap->h,
                                                          AV_PIX_FMT_RGBA,
                                                          sws_flags,
                                                          NULL,
                                                          NULL,
                                                          NULL);

      if (img_convert_ctx != NULL)
      {
        uint8_t* scaledpixels[1];
        scaledpixels[0] = malloc(bitmap->w * bitmap->h * 4);

        printf("converting...\n");

        uint8_t* pixels[4];
        int      pitch[4];

        pitch[0] = bitmap->w * 4;
        sws_scale(img_convert_ctx,
                  (const uint8_t* const*)frame->data,
                  frame->linesize,
                  0,
                  frame->height,
                  scaledpixels,
                  pitch);

        if (bitmap)
        {
          gfx_insert_rgb(bitmap,
                         scaledpixels[0],
                         bitmap->w,
                         bitmap->h,
                         0,
                         0);
        }
      }
    }
  }
}

bm_t* editor_get_image(const char* path)
{
  int i, ret = 0;

  AVFormatContext* src_ctx = avformat_alloc_context();

  /* // open the specified path */
  if (avformat_open_input(&src_ctx, path, NULL, NULL) != 0)
  {
    printf("avformat_open_input() failed");
  }

  // find the first attached picture, if available
  for (i = 0; i < src_ctx->nb_streams; i++)
  {
    if (src_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      AVCodecParameters* param = src_ctx->streams[i]->codecpar;

      const AVCodec*  codec        = avcodec_find_decoder(param->codec_id);
      AVCodecContext* codecContext = avcodec_alloc_context3(codec);

      avcodec_parameters_to_context(codecContext, param);
      avcodec_open2(codecContext, codec, NULL);

      AVPacket* packet = av_packet_alloc();
      AVFrame*  frame  = av_frame_alloc();

      while (av_read_frame(src_ctx, packet) >= 0)
      {
        avcodec_send_packet(codecContext, packet);
      }

      avcodec_receive_frame(codecContext, frame);

      static unsigned sws_flags = SWS_BICUBIC;

      struct SwsContext* img_convert_ctx = sws_getContext(frame->width,
                                                          frame->height,
                                                          frame->format,
                                                          frame->width,
                                                          frame->height,
                                                          AV_PIX_FMT_RGBA,
                                                          sws_flags,
                                                          NULL,
                                                          NULL,
                                                          NULL);

      if (img_convert_ctx != NULL)
      {
        bm_t* bitmap = bm_new(frame->width, frame->height);

        uint8_t* scaledpixels[1];
        scaledpixels[0] = malloc(bitmap->w * bitmap->h * 4);

        uint8_t* pixels[4];
        int      pitch[4];

        pitch[0] = bitmap->w * 4;
        sws_scale(img_convert_ctx,
                  (const uint8_t* const*)frame->data,
                  frame->linesize,
                  0,
                  frame->height,
                  scaledpixels,
                  pitch);

        gfx_rect(bitmap, 0, 0, bitmap->w, bitmap->h, 0xFF0000FF, 0);

        gfx_insert_rgba(bitmap,
                        scaledpixels[0],
                        bitmap->w,
                        bitmap->h,
                        0,
                        0);

        return bitmap;
      }
    }
  }

  return NULL;
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
