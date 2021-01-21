#ifndef config_h
#define config_h

#include "mtmap.c"

void config_read(map_t* db);
void config_write(map_t* db);
void config_init(map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "lib.c"
#include "mtcstring.c"
#include "mtlog.c"

// TODO rework kvlist, entries should contain a key field which is a hierarchical path so multi-level maps can be stored
void config_read(map_t* db)
{
  char* path = cstr_fromformat("%s/.config/zenmusic/config.kvl", getenv("HOME"), NULL);

  map_t* container = MNEW();
  kvlist_read(path, container, "id");

  printf("container\n");
  mem_describe(container, 0);

  if (container->count > 0)
  {
    map_t* cfdb = MGET(container, "config");
    vec_t* keys = VNEW();
    map_keys(cfdb, keys);

    for (int index = 0; index < keys->length; index++)
    {
      char* key = keys->data[index];
      MPUT(db, key, MGET(cfdb, key));
    }
  }

  printf("config loaded from %s, entries : %i\n", path, db->count);
  REL(path);
  REL(container);
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

  map_t* container = MNEW();
  MPUT(db, "id", cstr_fromcstring("config"));
  MPUT(container, "id", db);

  char* dirpath = cstr_fromformat("%s/.config/zenmusic/", getenv("HOME"), NULL);
  char* cfgpath = cstr_fromformat("%s/.config/zenmusic/config.kvl", getenv("HOME"), NULL);

  // TODO move mkpath to some other namespace
  int error = lib_mkpath(dirpath, 0777);

  if (error == 0)
  {
    int res = kvlist_write(cfgpath, container);
    if (res < 0) LOG("ERROR config_write cannot write config\n");
  }
  else
    LOG("ERROR config_write cannot create config path\n");

  REL(dirpath);
  REL(cfgpath);
  REL(container);
}

#endif
