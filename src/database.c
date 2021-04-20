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

struct _db_t
{
  map_t* data;
} db = {0};

void db_init()
{
  db.data = MNEW();
}

void db_reset()
{
  map_reset(db.data);
}

map_t* db_get_db()
{
  return db.data;
}

uint32_t db_count()
{
  return db.data->count;
}

void db_read(char* libpath)
{
  char* dbpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/zenmusic.kvl", libpath);

  LOG("reading db %s", dbpath);

  kvlist_read(dbpath, db.data, "file/path");
  REL(dbpath);

  LOG("database loaded, entries : %i", db.data->count);
}

void db_write(char* libpath)
{
  char* dbpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/zenmusic.kvl", libpath);

  LOG("writing db to %s", dbpath);

  int res = kvlist_write(dbpath, db.data);
  REL(dbpath);

  if (res < 0) LOG("ERROR db_write cannot write database %s\n", dbpath);

  LOG("db written");
}

void db_add_entry(char* path, map_t* entry)
{
  MPUT(db.data, path, entry);
}

#endif
