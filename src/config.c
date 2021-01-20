#ifndef config_h
#define config_h

#include "mtmap.c"

void config_read(map_t* db);
void config_write(map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "mtcstring.c"
#include "mtlog.c"

void config_read(map_t* db)
{
  kvlist_read("~/.config/zenmusic/config.kvl", db, "path");

  printf("config loaded, entries : %i\n", db->count);
}

void config_init(map_t* db)
{
  printf("config_init\n");

  MPUT(db, "organize_db", cstr_fromcstring("false"));
  MPUT(db, "ui_color", cstr_fromcstring("0xEEEEEEFF"));

  // we won't release values this time, config map will stay in memory during runtime
}

void config_write(map_t* db)
{
  printf("config_write\n");

  int res = kvlist_write("~/.config/zenmusic/config.kvl", db);

  if (res < 0) LOG("ERROR config_write cannot write config\n");
}

#endif
