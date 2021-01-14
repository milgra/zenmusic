#ifndef db_h
#define db_h

#include "mtmap.c"

void db_read(char* libpath, map_t* db);
void db_write(char* libpath, map_t* db);
void db_sort(map_t* db, vec_t* vec, char* field);
void db_filter(map_t* db, char* text, vec_t* vec);
void db_genres(map_t* db, vec_t* vec);
void db_artists(vec_t* vec, vec_t* res);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtlog.c"

void db_read(char* libpath, map_t* db)
{
  LOG("reading db %s", libpath);

  char* dbpath = cstr_fromformat("%s/zmusdb", libpath, NULL);
  char* dbstr  = cstr_fromfile(dbpath);

  if (dbstr)
  {
    char*  token = strtok(dbstr, "\n");
    char*  key   = NULL;
    map_t* map   = MNEW();

    while (token)
    {
      if (key)
      {
        char* val = cstr_fromcstring(token);
        MPUT(map, key, val);
        REL(key);
        REL(val);
        key = NULL;
      }
      else
      {
        if (token[0] == '-')
        {
          char* path = MGET(map, "path");
          MPUT(db, path, map);
          REL(map);
          map = MNEW();
        }
        else
          key = cstr_fromcstring(token);
      }
      token = strtok(NULL, "\n");
    }

    REL(dbstr);
  }
  else
  {
    printf("LOG No db found.\n");
  }

  REL(dbpath);

  LOG("database loaded, entries : %i", db->count);
}

void db_write(char* libpath, map_t* db)
{
  LOG("writing db to %s", libpath);

  vec_t* vals = VNEW();
  map_values(db, vals);

  char* new_path = cstr_fromformat("%s/zmusdb_new", libpath, NULL);
  char* old_path = cstr_fromformat("%s/zmusdb", libpath, NULL);

  FILE* f = fopen(new_path, "w");

  for (int index = 0; index < vals->length; index++)
  {
    map_t* entry = vals->data[index];
    vec_t* keys  = VNEW();
    map_keys(entry, keys);
    char* key;
    while ((key = VNXT(keys)))
    {
      char* val = MGET(entry, key);
      fprintf(f, "%s\n", key);
      fprintf(f, "%s\n", val);
    }
    fprintf(f, "-\n");
    REL(keys);
  }

  // TODO check file operations, don't rename db if failed
  fclose(f);
  REL(vals);

  // after successful write, copy new db to main db

  int succ = rename(new_path, old_path);

  // cleanup

  REL(vals);
  REL(new_path);
  REL(old_path);
}

char* sort_field = NULL;

int db_comp_artist(void* left, void* right)
{
  map_t* l = left;
  map_t* r = right;

  char* la = MGET(l, sort_field);
  char* ra = MGET(r, sort_field);

  return strcmp(la, ra);
}

vec_t* vec1 = NULL;
vec_t* vec2 = NULL;

void db_filter(map_t* db, char* text, vec_t* res)
{
  int ei, vi; // entry, value index

  if (!vec1) vec1 = VNEW();
  if (!vec2) vec2 = VNEW();

  vec_reset(res);
  vec_reset(vec1);
  vec_reset(vec2);

  map_values(db, vec1);

  for (ei = 0;
       ei < vec1->length;
       ei++)
  {
    map_t* entry = vec1->data[ei];
    vec_reset(vec2);
    map_values(entry, vec2);

    for (vi = 0;
         vi < vec2->length;
         vi++)
    {
      char* val = vec2->data[vi];
      if (strstr(val, text))
      {
        vec_add(res, entry);
        break;
      }
    }
  }
  vec_sort(res, db_comp_artist);
}

void db_sort(map_t* db, vec_t* vec, char* field)
{
  if (field != NULL)
  {
    sort_field = field;
    vec_reset(vec);
    map_values(db, vec);
    vec_sort(vec, db_comp_artist);
  }
}

void db_genres(map_t* db, vec_t* res)
{
  // TODO use map like in db_artists
  int ei, gi;

  if (!vec1) vec1 = VNEW();

  vec_reset(res);
  vec_reset(vec1);

  map_values(db, vec1);

  for (ei = 0;
       ei < vec1->length;
       ei++)
  {
    map_t* entry = vec1->data[ei];
    char*  genre = MGET(entry, "genre");

    if (genre)
    {
      char found = 0;
      for (gi = 0; gi < res->length; gi++)
      {
        char* act_genre = res->data[gi];
        if (strcmp(genre, act_genre) == 0)
        {
          found = 1;
          break;
        }
      }
      if (!found) VADD(res, genre);
    }
  }
}

void db_artists(vec_t* vec, vec_t* res)
{
  int ei, gi;

  map_t* artists = MNEW();

  for (ei = 0;
       ei < vec->length;
       ei++)
  {
    map_t* entry  = vec->data[ei];
    char*  artist = MGET(entry, "artist");

    if (artist) MPUT(artists, artist, artist);
  }

  map_values(artists, res);

  REL(artists);
}

#endif
