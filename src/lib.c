#ifndef lib_h
#define lib_h

#include "mtmap.c"

void lib_read();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtchannel.c"
#include "mtcstring.c"
#include <ftw.h>
#include <limits.h>

mtmap_t* lib_db;

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
  // remove entries existing in db
  mtvec_t* vals        = mtmap_values(db);
  mtmap_t* path_to_map = MNEW();
  for (int index = 0; index < vals->length; index++)
  {
    mtmap_t* map  = vals->data[index];
    char*    path = MGET(map, "path");
    MPUT(path_to_map, path, map);
  }
  REL(vals);
  // go through lib
  mtvec_t* keys = mtmap_keys(lib_db);
  for (int index = 0; index < keys->length; index++)
  {
    char*    key = keys->data[index];
    mtmap_t* map = MGET(path_to_map, key);
    if (map)
    {
      // path exist in db, removing entry from lib
      MDEL(lib_db, key);
    }
  }
  printf("LOG analyzing %i entries\n", lib_db->count);
}

void analyze_thread(mtch_t* channel)
{
  /* while (lib_db->count > 0) */
  /* { */
  /*   // remove last item */
  /*   if (mtch_send(map)) */
  /*   { */
  /*     // if sent, remove from lib_db */
  /*     RET(map) // retain because we will remove it from lib db and mtch doesn't retain */
  /*     mtvec_rematindex(lib_db, ); */
  /*   } */
  /* } */
}

// analyze in background thread
void lib_analyze(mtch_t* channel)
{

  /* // build up database */
  /* for (int index = 0; index < 100; index++) */
  /* { */
  /*   mtmap_t* map = files->data[index]; */

  /*   char idstr[10] = {0}; */
  /*   snprintf(idstr, 10, "%i", id++); */
  /*   char* idcstr = mtcstr_fromcstring(idstr); */

  /*   MPUT(map, "id", idcstr); */

  /*   char* path = MGET(map, "path"); */

  /*   player_get_metadata(path, map); */

  /*   if (MGET(map, "title") == NULL) MPUT(map, "title", path); */
  /*   if (MGET(map, "artist") == NULL) MPUT(map, "artist", path); */

  /*   MPUT(db, idstr, map); */
  /* } */
  /* // printf("FINAL:\n"); */
  /* // mtmem_describe(db, 0); */

  /* sorted = mtmap_values(db); */

  /* printf("sort\n"); */
  /* mtvec_sort(sorted, comp_artist); */

  /* printf("write\n"); */
  /* db_write(db); */
}

#endif
