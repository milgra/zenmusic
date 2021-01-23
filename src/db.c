#ifndef db_h
#define db_h

#include "mtmap.c"

void db_init();
void db_read(char* libpath);
void db_write(char* libpath);
void db_sort(char* field);
void db_filter(char* text, vec_t* vec);
void db_genres(vec_t* vec, vec_t* res);
void db_artists(vec_t* vec, vec_t* res);
void db_add_entry(char* path, map_t* entry);

map_t*   db_get_db();
vec_t*   db_get_songs();
vec_t*   db_get_genres();
vec_t*   db_get_artists();
uint32_t db_count();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "mtcstring.c"
#include "mtlog.c"

struct _db_t
{
  map_t* db;
  vec_t* songs;
  vec_t* genres;
  vec_t* artists;
} db = {0};

map_t* db_get_db()
{
  return db.db;
}

vec_t* db_get_songs()
{
  return db.songs;
}

vec_t* db_get_genres()
{
  return db.genres;
}

vec_t* db_get_artists()
{
  return db.artists;
}

uint32_t db_count()
{
  return db.db->count;
}

void db_init()
{
  db.db      = MNEW();
  db.songs   = VNEW();
  db.genres  = VNEW();
  db.artists = VNEW();
}

void db_read(char* libpath)
{
  char* dbpath = cstr_fromformat("%s/zenmusic.kvl", libpath, NULL);

  LOG("reading db %s", dbpath);

  kvlist_read(dbpath, db.db, "path");
  REL(dbpath);

  LOG("database loaded, entries : %i", db.db->count);
}

void db_write(char* libpath)
{
  char* dbpath = cstr_fromformat("%s/zenmusic.kvl", libpath, NULL);

  LOG("writing db to %s", dbpath);

  int res = kvlist_write(dbpath, db.db);
  REL(dbpath);

  if (res < 0) LOG("ERROR db_write cannot write database %s\n", dbpath);

  LOG("db written");
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

void db_filter(char* text, vec_t* res)
{
  int ei, vi; // entry, value index

  if (!vec1) vec1 = VNEW();
  if (!vec2) vec2 = VNEW();

  vec_reset(res);
  vec_reset(vec1);
  vec_reset(vec2);

  map_values(db.db, vec1);

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

void db_genres(vec_t* vec, vec_t* res)
{
  int ei, gi;

  for (ei = 0;
       ei < vec->length;
       ei++)
  {
    map_t* entry = vec->data[ei];
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

void db_sort_inner(vec_t* vec, char* field)
{
  if (field != NULL)
  {
    sort_field = field;
    vec_reset(vec);
    map_values(db.db, vec);
    vec_sort(vec, db_comp_artist);
  }
}

void db_sort(char* field)
{
  db_sort_inner(db.songs, field);
  db_genres(db.songs, db.genres);
  db_artists(db.songs, db.artists);
}

void db_add_entry(char* path, map_t* entry)
{
  MPUT(db.db, path, entry);
  VADD(db.songs, entry); // add immediately, needed on lib analyze to show partial results
}
#endif
