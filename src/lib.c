#ifndef lib_h
#define lib_h

#include "mtchannel.c"
#include "mtmap.c"

void lib_read();
void lib_remove_duplicates(map_t* db);
void lib_analyze(ch_t* channel);
int  lib_organize(map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "player.c"
#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

map_t* lib_db;
vec_t* rem_db;
char   lock_db = 0;

static int lib_file_data(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
{
  /* printf("%-3s %2d %7jd   %-40s %d %s\n", */
  /*        (tflag == FTW_D) ? "d" : (tflag == FTW_DNR) ? "dnr" : (tflag == FTW_DP) ? "dp" : (tflag == FTW_F) ? "f" : (tflag == FTW_NS) ? "ns" : (tflag == FTW_SL) ? "sl" : (tflag == FTW_SLN) ? "sln" : "???", */
  /*        ftwbuf->level, */
  /*        (intmax_t)sb->st_size, */
  /*        fpath, */
  /*        ftwbuf->base, */
  /*        fpath + ftwbuf->base); */

  if (tflag != FTW_D)
  {
    // TODO use macro for database name
    if (strstr(fpath, "zmusdb") == NULL)
    {
      char sizestr[20] = {0};
      snprintf(sizestr, 20, "%li", sb->st_size);
      char* size = cstr_fromcstring(sizestr);

      MPUT(lib_db, fpath, size);
      REL(size);
    }
  }

  return 0; /* To tell nftw() to continue */
}

void lib_read()
{
  if (lock_db) return;

  lib_db    = MNEW();
  int flags = 0;
  int id    = 0;
  //flags |= FTW_DEPTH;
  flags |= FTW_PHYS;

  nftw("/usr/home/milgra/Testmusic", lib_file_data, 20, flags);

  printf("LOG lib read, %i entries found\n", lib_db->count);
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

  printf("entries not in db %i\n", lib_db->count);

  REL(paths);
}

int analyzer_thread(void* chptr)
{
  ch_t* channel = chptr;
  printf("analyzer thread start\n");
  map_t* curr = NULL;

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

      player_get_metadata(path, curr);

      if (MGET(curr, "artist") == NULL) MPUT(curr, "artist", cstr_fromcstring("Unknown"));
      if (MGET(curr, "album") == NULL) MPUT(curr, "album", cstr_fromcstring("Unknown"));
      if (MGET(curr, "title") == NULL)
      {
        int index;
        for (index = strlen(path) - 1; index > -1; --index)
        {
          if (path[index] == '/')
          {
            index++;
            break;
          }
        }

        int   len   = strlen(path) - index;
        char* title = mem_calloc(len + 1, "char*", NULL, NULL);
        memcpy(title, path + index, len);

        MPUT(curr, "title", title);
      }
      // remove entry from remaining
      vec_rematindex(rem_db, rem_db->length - 1);
      // try to send it to main thread
      if (ch_send(channel, curr)) curr = NULL;
    }
    else
    {
      // wait for main thread to process new entries
      SDL_Delay(5);
      if (ch_send(channel, curr)) curr = NULL;
    }
  }

  printf("closing analyzer loop\n");
  lock_db = 0;
  return 0;
}

void lib_analyze(ch_t* channel)
{
  if (lock_db) return;

  lock_db = 1;

  rem_db = VNEW();
  map_keys(lib_db, rem_db);

  SDL_CreateThread(analyzer_thread, "analyzer", channel);
}

int lib_mkpath(char* file_path, mode_t mode)
{
  printf("mkdir %s\n", file_path);
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

int lib_organize(map_t* db)
{
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
      char* artist = MGET(entry, "artist");
      char* album  = MGET(entry, "album");
      char* title  = MGET(entry, "title");

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

      char* new_dirs = cstr_fromformat("%s/%s/%s/", "/usr/home/milgra/Testmusic", artist, album, NULL);
      char* new_path = cstr_fromformat("%s/%s/%s/%s.%s", "/usr/home/milgra/Testmusic", artist, album, title, ext, NULL);

      if (strcmp(path, new_path) != 0)
      {
        printf("moving %s to %s\n", path, new_path);

        int error = lib_mkpath(new_dirs, 0777);

        if (error == 0)
        {
          error = rename(path, new_path);

          if (error == 0)
          {
            printf("updating path in db,\n");
            MPUT(entry, "path", new_path);
            MPUT(db, new_path, entry);
            MDEL(db, path);
            changed = 1;
          }
          else
            printf("cannot rename file %s %s %s\n", path, new_path, strerror(errno));
        }
        else
          printf("cannot create path %s\n", new_path);
      }
    }
  }

  return changed;
}

#endif
