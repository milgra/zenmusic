#ifndef coder_h
#define coder_h

#include "zc_bitmap.c"
#include "zc_map.c"
#include "zc_vector.c"

void  coder_update_metadata(char* libpath, vec_t* songs, map_t* data, vec_t* drop, char* cover);
int   coder_get_metadata(const char* path, map_t* map);
int   coder_set_metadata(map_t* data, char* img_path);
void  coder_get_album(const char* path, bm_t* bitmap);
void  coder_update_song_metadata(char* libpath, char* path, map_t* data, vec_t* drop, char* cover);
bm_t* coder_get_image(const char* path);
void  coder_load_image_into(const char* path, bm_t* bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "zc_cstring.c"
#include "zc_cstrpath.c"
#include "zc_graphics.c"
#include "zc_log.c"
#include "zc_memory.c"
#include <limits.h>

void coder_clone_song(char* libpath, char* path, char* cover_path);

void coder_update_metadata(char* libpath, vec_t* songs, map_t* data, vec_t* drop, char* cover)
{
  /* printf("coder update metadata for songs:\n"); */
  /* mem_describe(songs, 0); */
  /* printf("\ndata:\n"); */
  /* mem_describe(data, 0); */
  /* printf("\ndrop:\n"); */
  /* mem_describe(drop, 0); */
  /* printf("\ncover %s\n", cover); */

  for (int index = 0; index < songs->length; index++)
  {
    printf("UPDATE SONGS %i INDEX %i\n", songs->length, index);

    map_t* song = songs->data[index];
    char*  path = MGET(song, "file/path");

    coder_clone_song(libpath, path, "/home/milgra/Projects/zenmusic/svg/freebsd.png");

    // coder_update_song_metadata(libpath, path, data, drop, cover);
  }
}

int coder_get_metadata(const char* path, map_t* map)
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
      AVOutputFormat* format = av_guess_format(NULL, path, NULL);
      if (format)
      {
        char* slash = strstr(format->mime_type, "/");

        if (slash)
        {
          char* media_type = mem_calloc(strlen(format->mime_type), "char*", NULL, NULL);
          char* container  = mem_calloc(strlen(format->mime_type), "char*", NULL, NULL);

          memcpy(media_type, format->mime_type, slash - format->mime_type);
          memcpy(container, slash + 1, strlen(format->mime_type) - (slash - format->mime_type));

          MPUT(map, "file/media_type", media_type);
          MPUT(map, "file/container", container);
          REL(media_type);
          REL(container);
        }
      }
      else
      {
        printf("cannot guess format for %s\n", path);
        return retv;
      }

      if (strcmp(MGET(map, "file/media_type"), "audio") != 0 && strcmp(MGET(map, "file/media_type"), "video") != 0)
      {
        printf("not audio not video\n");
        return retv;
      }

      retv = 0;

      av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

      AVDictionaryEntry* tag = NULL;

      while ((tag = av_dict_get(pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
      {
        char* value = cstr_fromcstring(tag->value);
        char* key   = cstr_fromformat(100, "%s/%s", "meta", tag->key);
        MPUT(map, key, value);
        REL(key);
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
        printf("coder_get_metadata no stream information found!!!\n");
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
      LOG("coder : skpping %s, no media context present", path);
    }

    avformat_close_input(&pFormatCtx);
  }
  else
  {
    LOG("coder : skipping %s, probably not a media file", path);
  }

  return retv;
}

void coder_update_song_metadata(char* libpath, char* path, map_t* data, vec_t* drop, char* cover_image)
{
  LOG("coder_update_song_metadata for %s %s\n", path, cover_image);

  // create temporary name

  char* tmp;
  char* dot = strrchr(path, '.');
  if (dot)
  {
    int   length = dot - path;
    char* name   = mem_calloc(length, "char*", NULL, NULL);
    memcpy(name, path, length);
    tmp = cstr_fromformat(PATH_MAX + NAME_MAX, "%s_tmp%s", name, dot);
  }
  else
  {
    tmp = cstr_fromformat(100, "%s_tmp", path);
  }

  char* oldpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", libpath, path);
  char* newpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", libpath, tmp);

  printf("oldpath %s\n", oldpath);
  printf("newpath %s\n", newpath);

  // open cover art first

  int res;

  AVPacket*          cover_packet  = NULL;
  AVFormatContext*   cover_context = NULL;
  AVCodecParameters* cover_cparam  = NULL;

  if (cover_image)
  {
    LOG("coder_set_metadata opening image file %s\n", cover_image);

    cover_context = avformat_alloc_context();
    res           = avformat_open_input(&cover_context, cover_image, 0, 0);

    if (res >= 0)
    {
      printf("cover avformat opened\n");

      res = avformat_find_stream_info(cover_context, 0);

      if (res >= 0)
      {
        printf("cover stream info found\n");

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
        LOG("ERROR coder_set_metadata cannot find stream info : %s\n", cover_image);
    }
    else
      LOG("ERROR coder_set_metadata cannot open image file : %s\n", cover_image);
  }

  // open source file

  AVFormatContext* src_ctx = avformat_alloc_context();

  res = avformat_open_input(&src_ctx, oldpath, 0, 0);

  if (res >= 0)
  {
    printf("input file opened\n");

    res = avformat_find_stream_info(src_ctx, 0);

    if (res >= 0)
    {
      printf("stream info found\n");

      AVFormatContext* out_ctx;
      AVOutputFormat*  out_fmt = av_guess_format(NULL, tmp, NULL);

      res = avformat_alloc_output_context2(&out_ctx, out_fmt, NULL, newpath);

      if (res >= 0)
      {
        printf("output context alloced\n");

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
                printf("Video Codec: resolution %d x %d\n", param->width, param->height);
              }
              else if (param->codec_type == AVMEDIA_TYPE_AUDIO)
              {
                printf("Audio Codec: %d channels, sample rate %d\n", param->channels, param->sample_rate);
              }
              printf("Codec %s ID %d bit_rate %ld\n", codec->long_name, codec->id, param->bit_rate);

              AVStream* ostream = avformat_new_stream(out_ctx, codec);

              avcodec_parameters_copy(ostream->codecpar, src_ctx->streams[i]->codecpar);

              ostream->codecpar->codec_tag = 0;
            }
          }
          else
            printf("skipping cover art stream\n");
        }

        // copy metadata in old file to new file

        av_dict_copy(&out_ctx->metadata, src_ctx->metadata, 0);

        AVDictionaryEntry* tag = NULL;

        printf("existing tags:\n");
        while ((tag = av_dict_get(out_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) printf("%s : %s\n", tag->key, tag->value);

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

          avio_open(&out_ctx->pb, newpath, AVIO_FLAG_WRITE);

          printf("new path opened\n");

          res = avformat_init_output(out_ctx, NULL);

          if (res > 0)
          {
            printf("avformat output inited\n");

            // create cover art image stream before writing header

            if (cover_cparam)
            {
              AVCodec* codec = avcodec_find_encoder(cover_cparam->codec_id);
              if (codec)
              {
                AVStream* ostream = avformat_new_stream(out_ctx, codec);

                cover_packet->stream_index = ostream->index;

                printf("creating new stream for new cover, index : %i\n", ostream->index);
                avcodec_parameters_copy(ostream->codecpar, cover_cparam);

                ostream->codecpar->codec_tag = cover_cparam->codec_tag;
                ostream->disposition |= AV_DISPOSITION_ATTACHED_PIC;
              }
            }

            res = avformat_write_header(out_ctx, NULL);

            if (res >= 0)
            {
              printf("header written\n");

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
                  src_pkt->stream_index = 0;
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

                if (res < 0) LOG("ERROR : coder_set_metadata : cannot write cover art image packet\n");

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
                LOG("ERROR : coder_set_metadata : cannot rename new file to old name\n");
              else
                printf("UPDATE SUCCESS\n");
            }
            else
              LOG("ERROR : coder_set_metadata : cannot write header\n");
          }
          else
            LOG("ERROR : coder_set_metadata : cannot init output\n");

          avio_close(out_ctx->pb);
        }
        else
          LOG("ERROR : coder_set_metadata : avformat needs no file\n");
      }
      else
        LOG("ERROR : coder_set_metadata : cannot allocate output context\n");
    }
    else
      LOG("ERROR : coder_set_metadata : cannot find stream info\n");
  }
  else
    LOG("ERROR : coder_set_metadata : cannot open input file\n");

  REL(tmp);
  REL(oldpath);
  REL(newpath);
}

void coder_clone_song(char* libpath, char* path, char* cover_path)
{
  LOG("coder_clone_song for %s cover %s\n", path, cover_path);

  char* ext  = cstr_path_extension(path); // REL 0
  char* name = cstr_path_filename(path);  // REL 1

  char* oldname = cstr_fromformat(PATH_MAX + NAME_MAX, "%s.%s", name, ext);        // REL 2
  char* oldpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", libpath, path);     // REL 3
  char* newpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s.tmp", libpath, path); // REL 4

  printf("oldpath %s\n", oldpath);
  printf("newpath %s\n", newpath);

  REL(name); // REL 1
  REL(ext);  // REL 0

  // open file for decode

  AVFormatContext* dec_ctx = avformat_alloc_context(); // FREE 0

  if (avformat_open_input(&dec_ctx, oldpath, 0, 0) >= 0)
  {
    printf("Input opened for decoding\n");

    if (avformat_find_stream_info(dec_ctx, 0) >= 0)
    {
      printf("Input stream info found, stream count %i\n", dec_ctx->nb_streams);

      AVFormatContext* enc_ctx;
      AVOutputFormat*  enc_fmt = av_guess_format(NULL, oldname, NULL);

      if (avformat_alloc_output_context2(&enc_ctx, enc_fmt, NULL, newpath) >= 0) // FREE 1
      {
        printf("Output context allocated\n");

        if (!(enc_ctx->oformat->flags & AVFMT_NOFILE))
        {
          printf("Output file will be provided by caller.\n");

          if (avio_open(&enc_ctx->pb, newpath, AVIO_FLAG_WRITE) >= 0) // CLOSE 0
          {
            printf("Output file created.\n");

            //
            // create all streams in the encoder context that are present in the decoder context
            //

            printf("Copying stream structure...\n");

            int dec_enc_strm[10]   = {0};
            int dec_cov_strm_index = -1;

            for (int si = 0; si < dec_ctx->nb_streams; si++)
            {
              AVCodecParameters* param = dec_ctx->streams[si]->codecpar;
              const AVCodec*     codec = avcodec_find_encoder(dec_ctx->streams[si]->codecpar->codec_id);
              int                cover = dec_ctx->streams[si]->disposition & AV_DISPOSITION_ATTACHED_PIC;

              if (codec)
              {
                printf("Stream no %i Codec %s ID %d bit_rate %ld\n", si, codec->long_name, codec->id, param->bit_rate);

                switch (param->codec_type)
                {
                case AVMEDIA_TYPE_VIDEO: printf("Video Codec: resolution %d x %d\n", param->width, param->height); break;
                case AVMEDIA_TYPE_AUDIO: printf("Audio Codec: %d channels, sample rate %d\n", param->channels, param->sample_rate); break;
                default: printf("Other codec: %i\n", param->codec_type); break;
                }

                if (cover_path && cover)
                {
                  dec_cov_strm_index = si;
                  printf("Skipping cover stream, index : %i\n", dec_cov_strm_index);
                }
                else
                {
                  // create stream in encoder context with codec

                  AVStream* enc_stream = avformat_new_stream(enc_ctx, codec);
                  avcodec_parameters_copy(enc_stream->codecpar, param);
                  enc_stream->codecpar->codec_tag = param->codec_tag; //  do we need this?

                  // store stream index mapping

                  dec_enc_strm[si] = enc_stream->index;

                  printf("Mapping decoder stream index %i to encoder stream index %i\n", si, enc_stream->index);
                }
              }
              else
                printf("No encoder found for stream no %i\n", si);
            }

            //
            // create cover art stream
            //

            AVFormatContext* cov_ctx       = NULL;
            int              cov_dec_index = 0;
            int              cov_enc_index = 0;

            if (cover_path)
            {
              cov_ctx = avformat_alloc_context(); // FREE 2

              if (avformat_open_input(&cov_ctx, cover_path, 0, 0) >= 0)
              {
                printf("Cover opened for decoding\n");

                if (avformat_find_stream_info(cov_ctx, 0) >= 0)
                {
                  printf("Cover stream info found, stream count %i\n", cov_ctx->nb_streams);

                  // find stream info

                  for (int si = 0; si < cov_ctx->nb_streams; si++)
                  {
                    AVCodecParameters* param = cov_ctx->streams[si]->codecpar;
                    const AVCodec*     codec = avcodec_find_encoder(cov_ctx->streams[si]->codecpar->codec_id);

                    if (codec)
                    {
                      printf("Cover stream no %i Codec %s ID %d bit_rate %ld\n", si, codec->long_name, codec->id, param->bit_rate);

                      switch (param->codec_type)
                      {
                      case AVMEDIA_TYPE_VIDEO: printf("Video Codec: resolution %d x %d\n", param->width, param->height); break;
                      case AVMEDIA_TYPE_AUDIO: printf("Audio Codec: %d channels, sample rate %d\n", param->channels, param->sample_rate); break;
                      default: printf("Other codec: %i\n", param->codec_type); break;
                      }

                      // create stream in encoder context with codec

                      if (param->codec_type == AVMEDIA_TYPE_VIDEO)
                      {
                        AVStream* enc_stream = avformat_new_stream(enc_ctx, codec);
                        avcodec_parameters_copy(enc_stream->codecpar, param);
                        enc_stream->codecpar->codec_tag = param->codec_tag; //  do we need this?
                        enc_stream->disposition |= AV_DISPOSITION_ATTACHED_PIC;

                        cov_dec_index = si;
                        cov_enc_index = enc_stream->index;

                        printf("Mapping cover stream index %i to encoder stream index %i\n", cov_dec_index, cov_enc_index);
                      }
                    }
                    else
                      printf("No encoder found for stream no %i\n", si);
                  }
                }
              }
            }

            if (avformat_init_output(enc_ctx, NULL) > 0)
            {
              printf("Output inited.\n");

              // write header

              if (avformat_write_header(enc_ctx, NULL) >= 0)
              {
                printf("Header written\n");

                // copy packets from decoder context to encoder context

                AVPacket* dec_pkt = av_packet_alloc();

                av_init_packet(dec_pkt);

                dec_pkt->data = NULL;
                dec_pkt->size = 0;

                // copy all packets from old file to new file with the exception of cover art image

                int last_di = -1; // last decoder index
                int last_ei = 0;  // last encoder index
                int last_pc = 0;  // last packet count
                int last_by = 0;  // last sum bytes

                while (av_read_frame(dec_ctx, dec_pkt) == 0)
                {
                  if (cover_path && dec_pkt->stream_index == dec_cov_strm_index)
                  {
                    printf("Skipping original cover packets.\n");
                  }
                  else
                  {

                    if (last_di == -1) last_di = dec_pkt->stream_index;

                    int enc_index = dec_enc_strm[dec_pkt->stream_index];

                    if (dec_pkt->stream_index == last_di)
                    {
                      last_by += dec_pkt->size;
                      last_pc += 1;
                      last_ei = enc_index;
                    }
                    else
                    {
                      printf("Stream written, dec index : %i enc index : %i packets : %i sum : %i bytes\n", last_di, last_ei, last_pc, last_by);
                      last_di = dec_pkt->stream_index;
                      last_ei = enc_index;
                      last_pc = 0;
                      last_by = 0;
                    }

                    dec_pkt->stream_index = enc_index;
                    av_write_frame(enc_ctx, dec_pkt);
                    dec_pkt->data = NULL;
                    dec_pkt->size = 0;
                  }
                }

                printf("Stream written, dec index : %i enc index : %i packets : %i sum : %i bytes\n", last_di, last_ei, last_pc, last_by);

                last_pc = 0;
                last_by = 0;

                if (cover_path)
                {
                  // write cover packets

                  while (av_read_frame(cov_ctx, dec_pkt) == 0)
                  {
                    if (dec_pkt->stream_index == cov_dec_index)
                    {
                      last_by += dec_pkt->size;
                      last_pc += 1;
                      dec_pkt->stream_index = cov_enc_index;
                      av_write_frame(enc_ctx, dec_pkt);
                    }

                    dec_pkt->data = NULL;
                    dec_pkt->size = 0;
                  }
                  printf("Cover stream written, dec index : %i enc index : %i packets : %i sum : %i bytes\n", cov_dec_index, cov_enc_index, last_pc, last_by);
                }

                // close output file and cleanup

                av_packet_free(&dec_pkt);
                av_write_trailer(enc_ctx);
              }
              else
                printf("Cannot write header.\n");
            }
            else
              printf("Cannot init output.\n");

            if (cov_ctx) avformat_free_context(cov_ctx);

            avio_close(enc_ctx->pb); // CLOSE 0
          }
          else
            printf("Cannot open file for encode %s\n", newpath);
        }
        else
          printf("Output is a fileless codec.");

        avformat_free_context(enc_ctx); // FREE 1
      }
      else
        printf("Cannot allocate output context for %s\n", oldname);
    }
    else
      printf("Cannot find stream info\n");

    avformat_close_input(&dec_ctx);
  }
  else
    printf("Cannot open file for decode %s\n", oldpath);

  avformat_free_context(dec_ctx); // FREE 0

  REL(oldname); // REL 2
  REL(oldpath); // REL 3
  REL(newpath); // REL 4
}

void coder_get_album(const char* path, bm_t* bitmap)
{
  assert(path != NULL);

  printf("coder_get_album %s %i %i\n", path, bitmap->w, bitmap->h);

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

bm_t* coder_get_image(const char* path)
{
  int i, ret = 0;

  AVFormatContext* src_ctx = avformat_alloc_context();

  /* // open the specified path */
  if (avformat_open_input(&src_ctx, path, NULL, NULL) == 0)
  {
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

          sws_freeContext(img_convert_ctx);

          return bitmap;
        }
      }
    }
  }

  return NULL;
}

void coder_load_image_into(const char* path, bm_t* bitmap)
{
  int i, ret = 0;

  AVFormatContext* src_ctx = avformat_alloc_context();

  /* // open the specified path */
  if (avformat_open_input(&src_ctx, path, NULL, NULL) == 0)
  {
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

          sws_freeContext(img_convert_ctx);
        }
      }
    }
  }
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
