#ifndef db_h
#define db_h

#include "mtmap.c"

void db_init();
void db_read(char* libpath);
void db_write(char* libpath);
void db_add_entry(char* path, map_t* entry);
void db_remove_entry(map_t* entry);
void db_update(map_t* map);

int db_organize_entry(char* libpath, map_t* db, map_t* entry);
int db_organize(char* libpath, map_t* db);

map_t*   db_get_db();
uint32_t db_count();
void     db_reset();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "library.c"
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

  LOG("db : reading db %s", dbpath);

  kvlist_read(dbpath, db, "file/path");

  LOG("db : loaded, entries : %i", db->count);

  REL(dbpath); // REL 0
}

void db_write(char* libpath)
{
  assert(libpath != NULL);

  char* dbpath = cstr_fromformat(PATH_MAX + NAME_MAX, "/%s/zenmusic.kvl", libpath);

  int res = kvlist_write(dbpath, db);

  if (res < 0) LOG("ERROR db_write cannot write database %s\n", dbpath);

  LOG("db : written");

  REL(dbpath);
}

void db_add_entry(char* path, map_t* entry)
{
  MPUT(db, path, entry);
}

void db_remove_entry(map_t* entry)
{
  char* path = MGET(entry, "file/path");
  MDEL(db, path);
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

void db_update(map_t* files)
{
  // first remove deleted files from db
  vec_t* paths = VNEW();
  map_keys(db, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(files, path);
    if (!map)
    {
      // db path is missing from file path, file was removed
      MDEL(db, path);
      printf("LOG file is missing for path %s, song entry was removed from db\n", path);
    }
  }

  // go through lib paths
  vec_reset(paths);
  map_keys(files, paths);
  for (int index = 0; index < paths->length; index++)
  {
    char*  path = paths->data[index];
    map_t* map  = MGET(db, path);
    if (map)
    {
      // path exist in db, removing entry from files
      MDEL(files, path);
    }
  }
}

char* db_replace_char(char* str, char find, char replace)
{
  char* current_pos = strchr(str, find);
  while (current_pos)
  {
    *current_pos = replace;
    current_pos  = strchr(current_pos + 1, find);
  }
  return str;
}

int db_organize_entry(char* libpath, map_t* db, map_t* entry)
{
  assert(libpath != NULL);

  int changed = 0;

  char* path   = MGET(entry, "file/path");
  char* artist = MGET(entry, "meta/artist");
  char* album  = MGET(entry, "meta/album");
  char* title  = MGET(entry, "meta/title");
  char* track  = MGET(entry, "meta/track");

  // remove slashes before directory creation

  db_replace_char(artist, '/', ' ');
  db_replace_char(title, '/', ' ');

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
    int trackno  = atoi(track);
    new_path     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s/%s/%.3i %s.%s", libpath, artist, album, trackno, title, ext);
    new_path_rel = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s/%.3i %s.%s", artist, album, trackno, title, ext);
  }
  else
  {
    new_path     = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s/%s/%s.%s", libpath, artist, album, title, ext);
    new_path_rel = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s/%s.%s", artist, album, title, ext);
  }

  if (strcmp(old_path, new_path) != 0)
  {
    int error = lib_rename_file(old_path, new_path, new_dirs);
    if (error == 0)
    {
      LOG("db : updating path");
      MPUT(entry, "file/path", new_path_rel);
      MPUT(db, new_path_rel, entry);
      MDEL(db, path);
      changed = 1;
    }
  }

  REL(new_dirs);
  REL(new_path);
  REL(new_path_rel);
  REL(old_path);

  return 0;
}

int db_organize(char* libpath, map_t* db)
{
  LOG("db : organizing database");

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
      changed |= db_organize_entry(libpath, db, entry);
    }
  }

  REL(paths);

  return changed;
}

#endif
