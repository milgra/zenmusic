#ifndef lib_h
#define lib_h

#include "zc_channel.c"
#include "zc_map.c"

void lib_read_files(char* libpath, map_t* db);
void lib_delete_file(char* libpath, map_t* entry);
int  lib_rename_file(char* old, char* new, char* new_dirs);
void lib_analyze_files(ch_t* channel, map_t* files);

#endif

#if __INCLUDE_LEVEL__ == 0

#define __USE_XOPEN_EXTENDED 1 // needed for linux
#include <ftw.h>

#include "editor.c"
#include "files.c"
#include "zc_cstring.c"
#include "zc_cstrpath.c"
#include "zc_log.c"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

static int lib_file_data_step(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf);
int        analyzer_thread(void* chptr);

struct lib_t
{
  map_t* files;
  vec_t* paths;
  char   lock;
  char*  path;
} lib = {0};

void lib_read_files(char* lib_path, map_t* files)
{
  assert(lib_path != NULL);

  lib.files = files;
  lib.path  = lib_path;

  nftw(lib_path, lib_file_data_step, 20, FTW_PHYS);

  LOG("lib : scanned, files : %i", files->count);
}

static int lib_file_data_step(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
{
  /* printf("%-3s %2d %7jd   %-40s %d %s\n", */
  /*        (tflag == FTW_D) ? "d" : (tflag == FTW_DNR) ? "dnr" : (tflag == FTW_DP) ? "dp" : (tflag == FTW_F) ? "f" : (tflag == FTW_NS) ? "ns" : (tflag == FTW_SL) ? "sl" : (tflag == FTW_SLN) ? "sln" : "???", */
  /*        ftwbuf->level, */
  /*        (intmax_t)sb->st_size, */
  /*        fpath, */
  /*        ftwbuf->base, */
  /*        fpath + ftwbuf->base); */

  if (tflag == FTW_F)
  {
    // TODO use macro for database name
    if (strstr(fpath, "zenmusic") == NULL)
    {
      char* size = cstr_fromformat(20, "%li", sb->st_size);
      MPUT(lib.files, fpath + strlen(lib.path), size); // use relative path as path
      REL(size);
    }
  }

  return 0; /* To tell nftw() to continue */
}

void lib_delete_file(char* lib_path, map_t* entry)
{
  assert(lib_path != NULL);

  char* rel_path  = MGET(entry, "file/path");
  char* file_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", lib_path, rel_path);

  int error = remove(file_path);
  if (error)
    LOG("lib : cannot remove file %s : %s", file_path, strerror(errno));
  else
    LOG("lib : file %s removed.", file_path);

  REL(file_path);
}

int lib_rename_file(char* old_path, char* new_path, char* new_dirs)
{
  LOG("lib : renaming %s to %s", old_path, new_path);

  int error = files_mkpath(new_dirs, 0777);

  if (error == 0)
  {
    error = rename(old_path, new_path);
    return error;
  }
  return error;
}

void lib_analyze_files(ch_t* channel, map_t* files)
{
  if (lib.lock) return;

  lib.lock  = 1;
  lib.files = RET(files);
  lib.paths = VNEW();

  map_keys(files, lib.paths);

  SDL_CreateThread(analyzer_thread, "analyzer", channel);
}

int analyzer_thread(void* chptr)
{
  ch_t*    channel = chptr;
  map_t*   song    = NULL;
  uint32_t total   = lib.paths->length;
  int      ratio   = -1;

  while (lib.paths->length > 0)
  {
    if (!song)
    {
      song = MNEW();

      char* path = vec_tail(lib.paths);
      char* size = MGET(lib.files, path);

      char* time_str = mem_calloc(80, "char*", NULL, NULL);
      // snprintf(time_str, 20, "%lu", time(NULL));

      time_t now;
      time(&now);
      struct tm ts = *localtime(&now);
      strftime(time_str, 80, "%Y-%m-%d %H:%M:%S", &ts);

      // add file data

      MPUT(song, "file/path", path);
      MPUT(song, "file/size", size);
      MPUT(song, "file/added", time_str);
      MPUT(song, "file/last_played", time_str);
      MPUT(song, "file/last_skipped", time_str);
      MPUT(song, "file/play_count", cstr_fromcstring("0"));
      MPUT(song, "file/skip_count", cstr_fromcstring("0"));

      char* real = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", lib.path, path);

      LOG("lib : analyzing %s", real);

      // read and add file and meta data

      int res = editor_get_metadata(real, song);

      if (res == 0)
      {
        if (MGET(song, "meta/artist") == NULL) MPUTR(song, "meta/artist", cstr_fromcstring("Unknown"));
        if (MGET(song, "meta/album") == NULL) MPUTR(song, "meta/album", cstr_fromcstring("Unknown"));
        if (MGET(song, "meta/title") == NULL) MPUTR(song, "meta/title", cstr_path_filename(path));

        // try to send it to main thread
        if (ch_send(channel, song)) song = NULL;
      }
      else
      {
        // file is not a media file readable by ffmpeg, we skip it
        REL(song);
        song = NULL;
      }

      // cleanup
      REL(real);

      // remove entry from remaining
      vec_rematindex(lib.paths, lib.paths->length - 1);
    }
    else
    {
      // wait for main thread to process new entries
      SDL_Delay(5);
      if (ch_send(channel, song)) song = NULL;
    }

    // show progress
    int ratio_new = (int)((float)(total - lib.paths->length) / (float)total * 100.0);
    if (ratio != ratio_new)
    {
      ratio = ratio_new;
      LOG(" lib : analyzer progress : %i%%", ratio);
    }
  }

  // send empty song to initiaite finish
  song = MNEW();
  MPUT(song, "file/path", cstr_fromcstring("//////")); // impossible path
  ch_send(channel, song);                              // send finishing entry

  REL(lib.files);
  REL(lib.paths);

  lib.files = NULL;
  lib.paths = NULL;

  lib.lock = 0;
  return 0;
}

#endif
