#ifndef db_h
#define db_h

#include "mtmap.c"

void db_init();
void db_read(char* libpath);
void db_write(char* libpath);
void db_add_entry(char* path, map_t* entry);

map_t*   db_get_db();
uint32_t db_count();
void     db_reset();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "mtcstring.c"
#include "mtlog.c"
#include "mtvector.c"
#include <ctype.h>
#include <limits.h>

map_t* db;

void db_init()
{
  db = MNEW();
}

void db_read(char* libpath)
{
  assert(libpath != NULL);

  char* dbpath = cstr_fromformat(PATH_MAX + NAME_MAX, "/%s/zenmusic.kvl", libpath); // REL 0

  LOG("reading db %s", dbpath);

  kvlist_read(dbpath, db, "file/path");

  LOG("database loaded, entries : %i", db->count);

  REL(dbpath); // REL 0
}

void db_write(char* libpath)
{
  assert(libpath != NULL);

  char* dbpath = cstr_fromformat(PATH_MAX + NAME_MAX, "/%s/zenmusic.kvl", libpath);

  LOG("writing db to %s", dbpath);

  int res = kvlist_write(dbpath, db);

  if (res < 0) LOG("ERROR db_write cannot write database %s\n", dbpath);

  LOG("db written");

  REL(dbpath);
}

void db_add_entry(char* path, map_t* entry)
{
  MPUT(db, path, entry);
}

void db_reset()
{
  map_reset(db);
}

map_t* db_get_db()
{
  return db;
}

uint32_t db_count()
{
  return db->count;
}

#endif
