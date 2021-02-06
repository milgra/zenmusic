#ifndef lib_h
#define lib_h

#include "mtchannel.c"
#include "mtmap.c"

void lib_read(char* libpath);
void lib_remove_duplicates(map_t* db);
void lib_analyze(ch_t* channel);
int  lib_organize_entry(char* libpath, map_t* db, map_t* entry);
int  lib_organize(char* libpath, map_t* db);
int  lib_entries();
int  lib_exists(char* path);
int  lib_mkpath(char* file_path, mode_t mode);

#endif

#if __INCLUDE_LEVEL__ == 0

#define __USE_XOPEN_EXTENDED 1 // needed for linux
#include <ftw.h>

#include "mtcstring.c"
#include "mtlog.c"
#include "player.c"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

map_t* lib_db;
vec_t* rem_db;
char   lock_db  = 0;
char*  lib_path = NULL;

int lib_entries()
{
  return lib_db->count;
}

static int lib_file_data(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
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
      char sizestr[20] = {0};
      snprintf(sizestr, 20, "%li", sb->st_size);
      char* size = cstr_fromcstring(sizestr);

      // printf("file %s\n", fpath + strlen(lib_path));

      MPUT(lib_db, fpath + strlen(lib_path), size);
      REL(size);
    }
  }

  return 0; /* To tell nftw() to continue */
}

int lib_exists(char* path)
{
  struct stat sb;

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    return 1;
  else
    return 0;
}

void lib_read(char* libpath)
{
  if (lock_db) return;

  if (lib_path) REL(lib_path);
  if (lib_db) REL(lib_db);

  lib_path  = cstr_fromcstring(libpath);
  lib_db    = MNEW();
  int flags = 0;
  int id    = 0;
  //flags |= FTW_DEPTH;
  flags |= FTW_PHYS;

  nftw(libpath, lib_file_data, 20, flags);

  LOG("library scanned, files : %i", lib_db->count);
}

void lib_remove_duplicates(map_t* db)
{
  if (lock_db) return;

  // go through db paths
  vec_t* paths = VNEW();
  map_keys(db, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(lib_db, path);
    if (!map)
    {
      // db path is missing from file path, file was removed
      MDEL(db, path);
      printf("LOG file is missing for path %s, song entry was removed from db\n", path);
    }
  }

  // go through lib paths
  vec_reset(paths);
  map_keys(lib_db, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(db, path);
    if (map)
    {
      // path exist in db, removing entry from lib
      MDEL(lib_db, path);
    }
  }

  REL(paths);

  LOG("new files detected : %i", lib_entries());
}

int analyzer_thread(void* chptr)
{
  ch_t*    channel = chptr;
  map_t*   curr    = NULL;
  uint32_t total   = rem_db->length;
  int      ratio   = -1;

  while (rem_db->length > 0)
  {
    if (!curr)
    {
      // create actual song entry
      curr       = MNEW();
      char* path = vec_tail(rem_db);
      char* size = MGET(lib_db, path);
      MPUT(curr, "path", path);
      MPUT(curr, "size", size);
      char* real = cstr_fromformat("%s%s", lib_path, path, NULL);

      int res = player_get_metadata(real, curr);
      REL(real);

      if (res == 0)
      {
        if (MGET(curr, "artist") == NULL) MPUT(curr, "artist", cstr_fromcstring("Unknown"));
        if (MGET(curr, "album") == NULL) MPUT(curr, "album", cstr_fromcstring("Unknown"));
        if (MGET(curr, "title") == NULL)
        {
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

            MPUT(curr, "title", title);
          }
          else
            MPUT(curr, "title", path);
        }
        // try to send it to main thread
        if (ch_send(channel, curr)) curr = NULL;
      }
      else
      {
        // file is not a media file readable by ffmpeg, we skip it
        REL(curr);
        curr = NULL;
      }
      // remove entry from remaining
      vec_rematindex(rem_db, rem_db->length - 1);
    }
    else
    {
      // wait for main thread to process new entries
      SDL_Delay(5);
      if (ch_send(channel, curr)) curr = NULL;
    }

    // show progress
    int ratio_new = (int)((float)(total - rem_db->length) / (float)total * 100.0);
    if (ratio != ratio_new)
    {
      ratio = ratio_new;
      LOG(" analyzer progress : %i%%", ratio);
    }
  }

  curr = MNEW();
  MPUT(curr, "path", cstr_fromcstring("//////")); // impossible path
  ch_send(channel, curr);                         // send finishing entry

  lock_db = 0;
  return 0;
}

void lib_analyze(ch_t* channel)
{
  if (lock_db) return;

  LOG("analyzing entires...");

  lock_db = 1;

  if (rem_db) REL(rem_db);

  rem_db = VNEW();
  map_keys(lib_db, rem_db);

  SDL_CreateThread(analyzer_thread, "analyzer", channel);
}

int lib_mkpath(char* file_path, mode_t mode)
{
  assert(file_path && *file_path);
  for (char* p = strchr(file_path + 1, '/');
       p;
       p = strchr(p + 1, '/'))
  {
    *p = '\0';
    if (mkdir(file_path, mode) == -1)
    {
      if (errno != EEXIST)
      {
        *p = '/';
        return -1;
      }
    }
    *p = '/';
  }
  return 0;
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
  int changed = 0;

  char* path   = MGET(entry, "path");
  char* artist = MGET(entry, "artist");
  char* album  = MGET(entry, "album");
  char* title  = MGET(entry, "title");
  char* track  = MGET(entry, "track");

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

  char* old_path     = cstr_fromformat("%s%s", libpath, path, NULL);
  char* new_dirs     = cstr_fromformat("%s%s/%s/", libpath, artist, album, NULL);
  char* new_path     = NULL;
  char* new_path_rel = NULL;

  if (track)
  {
    int  trackno    = atoi(track);
    char trackst[5] = {0};
    snprintf(trackst, 5, "%.3i", trackno);
    new_path     = cstr_fromformat("%s%s/%s/%s %s.%s", libpath, artist, album, trackst, title, ext, NULL);
    new_path_rel = cstr_fromformat("%s/%s/%s %s.%s", artist, album, trackst, title, ext, NULL);
  }
  else
  {
    new_path     = cstr_fromformat("%s%s/%s/%s.%s", libpath, artist, album, title, ext, NULL);
    new_path_rel = cstr_fromformat("%s/%s/%s.%s", artist, album, title, ext, NULL);
  }

  if (strcmp(old_path, new_path) != 0)
  {
    LOG("moving %s to %s\n", old_path, new_path);

    int error = lib_mkpath(new_dirs, 0777);

    if (error == 0)
    {
      error = rename(old_path, new_path);

      if (error == 0)
      {
        LOG("updating path in db,\n");
        MPUT(entry, "path", new_path_rel);
        MPUT(db, new_path_rel, entry);
        MDEL(db, path);
        changed = 1;
      }
      else
        LOG("cannot rename file %s %s %s\n", old_path, new_path, strerror(errno));
    }
    else
      LOG("cannot create path %s\n", new_path);
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
