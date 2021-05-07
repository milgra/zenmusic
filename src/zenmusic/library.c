#ifndef lib_h
#define lib_h

#include "mtchannel.c"
#include "mtmap.c"

void lib_read(char* libpath);
void lib_delete_file(char* libpath, map_t* entry);
void lib_analyze(ch_t* channel);

void lib_remove_duplicates(map_t* db);
int  lib_organize_entry(char* libpath, map_t* db, map_t* entry);
int  lib_organize(char* libpath, map_t* db);
int  lib_entries();
int  lib_exists(char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#define __USE_XOPEN_EXTENDED 1 // needed for linux
#include <ftw.h>

#include "editor.c"
#include "files.c"
#include "mtcstring.c"
#include "mtlog.c"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

static int lib_file_data_step(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf);
int        analyzer_thread(void* chptr);

struct lib_t
{
  map_t* db;
  vec_t* remaining;
  char   lock;
  char*  path;
} lib = {0};

void lib_read(char* lib_path)
{
  if (lib.lock) return;

  if (lib.path) REL(lib.path);
  if (lib.db) REL(lib.db);

  lib.path = cstr_fromcstring(lib_path);
  lib.db   = MNEW();

  nftw(lib_path, lib_file_data_step, 20, FTW_PHYS);

  LOG("library scanned, files : %i", lib.db->count);
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
      MPUT(lib.db, fpath + strlen(lib.path), size); // use relative path as path
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
    LOG("cannot remove file %s : %s\n", file_path, strerror(errno));
  else
    LOG("file %s removed.\n", file_path);

  REL(file_path);
}

void lib_analyze(ch_t* channel)
{
  if (lib.lock) return;

  lib.lock = 1;

  if (lib.remaining) REL(lib.remaining);
  lib.remaining = VNEW();

  map_keys(lib.db, lib.remaining);

  SDL_CreateThread(analyzer_thread, "analyzer", channel);
}

int analyzer_thread(void* chptr)
{
  ch_t*    channel = chptr;
  map_t*   song    = NULL;
  uint32_t total   = lib.remaining->length;
  int      ratio   = -1;

  while (lib.remaining->length > 0)
  {
    if (!song)
    {
      song = MNEW();

      char* path = vec_tail(lib.remaining);
      char* size = MGET(lib.db, path);

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

      // read and add file and meta data

      int res = editor_get_metadata(real, song);

      if (res == 0)
      {
        if (MGET(song, "meta/artist") == NULL) MPUT(song, "meta/artist", cstr_fromcstring("Unknown"));
        if (MGET(song, "meta/album") == NULL) MPUT(song, "meta/album", cstr_fromcstring("Unknown"));
        if (MGET(song, "meta/title") == NULL)
        {
          // if no title present use file name

          int dotindex;
          for (dotindex = strlen(path) - 1; dotindex > -1; --dotindex)
          {
            if (path[dotindex] == '.') break;
          }

          int slashindex;
          for (slashindex = strlen(path) - 1; slashindex > -1; --slashindex)
          {
            if (path[slashindex] == '/')
            {
              slashindex++;
              break;
            }
          }

          if (dotindex > slashindex)
          {
            int   len   = dotindex - slashindex;
            char* title = mem_calloc(len + 1, "char*", NULL, NULL);
            memcpy(title, path + slashindex, len);

            MPUT(song, "meta/title", title);
          }
          else
          {
            // use path if nothing works

            MPUT(song, "meta/title", path);
          }
        }

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
      vec_rematindex(lib.remaining, lib.remaining->length - 1);
    }
    else
    {
      // wait for main thread to process new entries
      SDL_Delay(5);
      if (ch_send(channel, song)) song = NULL;
    }

    // show progress
    int ratio_new = (int)((float)(total - lib.remaining->length) / (float)total * 100.0);
    if (ratio != ratio_new)
    {
      ratio = ratio_new;
      LOG(" analyzer progress : %i%%", ratio);
    }
  }

  // send empty song to initiaite finish
  song = MNEW();
  MPUT(song, "file/path", cstr_fromcstring("//////")); // impossible path
  ch_send(channel, song);                              // send finishing entry

  lib.lock = 0;
  return 0;
}

int lib_entries()
{
  return lib.db->count;
}

void lib_remove_duplicates(map_t* db)
{
  if (lib.lock) return;

  // go through db paths
  vec_t* paths = VNEW();
  map_keys(db, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(lib.db, path);
    if (!map)
    {
      // db path is missing from file path, file was removed
      MDEL(db, path);
      printf("LOG file is missing for path %s, song entry was removed from db\n", path);
    }
  }

  // go through lib paths
  vec_reset(paths);
  map_keys(lib.db, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(db, path);
    if (map)
    {
      // path exist in db, removing entry from lib
      MDEL(lib.db, path);
    }
  }

  REL(paths);

  LOG("new files detected : %i", lib_entries());
}

char* lib_replace_char(char* str, char find, char replace)
{
  char* current_pos = strchr(str, find);
  while (current_pos)
  {
    *current_pos = replace;
    current_pos  = strchr(current_pos + 1, find);
  }
  return str;
}

int lib_organize_entry(char* libpath, map_t* db, map_t* entry)
{
  assert(libpath != NULL);

  int changed = 0;

  char* path   = MGET(entry, "file/path");
  char* artist = MGET(entry, "meta/artist");
  char* album  = MGET(entry, "meta/album");
  char* title  = MGET(entry, "meta/title");
  char* track  = MGET(entry, "meta/track");

  // remove slashes before directory creation

  lib_replace_char(artist, '/', ' ');
  lib_replace_char(title, '/', ' ');

  // get extension

  int index;
  for (index = strlen(path) - 1; index > -1; --index)
  {
    if (path[index] == '.')
    {
      index++;
      break;
    }
  }

  int   len = strlen(path) - index;
  char* ext = mem_calloc(len + 1, "char*", NULL, NULL);
  memcpy(ext, path + index, len);

  char* old_path     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", libpath, path);
  char* new_dirs     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s/%s/", libpath, artist, album);
  char* new_path     = NULL;
  char* new_path_rel = NULL;

  if (track)
  {
    int  trackno    = atoi(track);
    char trackst[5] = {0};
    snprintf(trackst, 5, "%.3i", trackno);
    new_path     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s/%s/%s %s.%s", libpath, artist, album, trackst, title, ext);
    new_path_rel = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s/%s %s.%s", artist, album, trackst, title, ext);
  }
  else
  {
    new_path     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s/%s/%s.%s", libpath, artist, album, title, ext);
    new_path_rel = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s/%s.%s", artist, album, title, ext);
  }

  if (strcmp(old_path, new_path) != 0)
  {
    LOG("moving %s to %s\n", old_path, new_path);

    /* int error = files_mkpath(new_dirs, 0777); */

    /* if (error == 0) */
    /* { */
    /*   error = rename(old_path, new_path); */

    /*   if (error == 0) */
    /*   { */
    /*     LOG("updating path in db,\n"); */
    /*     MPUT(entry, "file/path", new_path_rel); */
    /*     MPUT(db, new_path_rel, entry); */
    /*     MDEL(db, path); */
    /*     changed = 1; */
    /*   } */
    /*   else */
    /*     LOG("cannot rename file %s %s %s\n", old_path, new_path, strerror(errno)); */
    /* } */
    /* else */
    /*   LOG("cannot create path %s\n", new_path); */
  }

  REL(new_dirs);
  REL(new_path);
  REL(new_path_rel);
  REL(old_path);

  return 0;
}

int lib_organize(char* libpath, map_t* db)
{
  LOG("organizing database...");

  // go through all db entries, check path, move if needed

  int    changed = 0;
  vec_t* paths   = VNEW();

  map_keys(db, paths);

  for (int index = 0; index < paths->length; index++)
  {
    char*  path  = paths->data[index];
    map_t* entry = MGET(db, path);

    if (entry)
    {
      changed |= lib_organize_entry(libpath, db, entry);
    }
  }

  REL(paths);

  return changed;
}

#endif
