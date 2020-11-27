#ifndef lib_h
#define lib_h

#include "mtchannel.c"
#include "mtmap.c"

void lib_read();
void lib_remove_duplicates(mtmap_t* db);
void lib_analyze(mtch_t* channel);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "player.c"
#include <ftw.h>
#include <limits.h>

mtmap_t* lib_db;
mtvec_t* rem_db;
char     lock_db = 0;

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
    char sizestr[20] = {0};
    snprintf(sizestr, 20, "%li", sb->st_size);
    char* size = mtcstr_fromcstring(sizestr);

    MPUT(lib_db, fpath, size);
    REL(size);
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

  nftw("/usr/home/milgra/Music", lib_file_data, 20, flags);

  printf("LOG lib read, %i entries found\n", lib_db->count);
}

void lib_remove_duplicates(mtmap_t* db)
{
  if (lock_db) return;

  // go through db paths
  mtvec_t* paths = mtmap_keys(db);
  for (int index = 0; index < paths->length; index++)
  {
    char*    path = paths->data[index];
    mtmap_t* map  = MGET(lib_db, path);
    if (!map)
    {
      // db path is missing from file path, file was removed
      MDEL(db, path);
      printf("LOG file is missing for path %s, song entry was removed from db\n", path);
    }
  }

  // go through lib paths
  paths = mtmap_keys(lib_db);
  for (int index = 0; index < paths->length; index++)
  {
    char*    path = paths->data[index];
    mtmap_t* map  = MGET(db, path);
    if (map)
    {
      // path exist in db, removing entry from lib
      MDEL(lib_db, path);
    }
  }

  REL(paths);
}

int analyzer_thread(void* chptr)
{
  mtch_t* channel = chptr;
  printf("analyzer thread start\n");
  mtmap_t* curr = NULL;

  while (rem_db->length > 0)
  {
    if (!curr)
    {
      // create actual song entry
      curr       = MNEW();
      char* path = mtvec_tail(rem_db);
      char* size = MGET(lib_db, path);
      MPUT(curr, "path", path);
      MPUT(curr, "size", size);
      if (MGET(curr, "title") == NULL) MPUT(curr, "title", path);
      if (MGET(curr, "artist") == NULL) MPUT(curr, "artist", path);
      player_get_metadata(path, curr);
      // remove entry from remaining
      mtvec_rematindex(rem_db, rem_db->length - 1);
      // try to send it to main thread
      if (mtch_send(channel, curr)) curr = NULL;
    }
    else
    {
      // wait for main thread to process new entries
      SDL_Delay(5);
      if (mtch_send(channel, curr)) curr = NULL;
    }
  }

  printf("closing analyzer loop\n");
  lock_db = 0;
  return 0;
}

void lib_analyze(mtch_t* channel)
{
  if (lock_db) return;

  lock_db = 1;
  rem_db  = mtmap_keys(lib_db);

  SDL_CreateThread(analyzer_thread, "analyzer", channel);
}

#endif
