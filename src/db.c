#ifndef db_h
#define db_h

#include "mtmap.c"

void db_init();
void db_read(char* libpath);
void db_write(char* libpath);
void db_sort(char* field);
void db_filter(char* text);
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
  map_t* data;
  vec_t* songs;
  vec_t* genres;
  vec_t* artists;

  char* sort_field;

  vec_t* tmp1;
  vec_t* tmp2;
} db = {0};

void db_init()
{
  db.data    = MNEW();
  db.songs   = VNEW();
  db.genres  = VNEW();
  db.artists = VNEW();

  db.tmp1 = VNEW();
  db.tmp2 = VNEW();
}

map_t* db_get_db()
{
  return db.data;
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
  return db.data->count;
}

void db_read(char* libpath)
{
  char* dbpath = cstr_fromformat("%s/zenmusic.kvl", libpath, NULL);

  LOG("reading db %s", dbpath);

  kvlist_read(dbpath, db.data, "path");
  REL(dbpath);

  LOG("database loaded, entries : %i", db.data->count);
}

void db_write(char* libpath)
{
  char* dbpath = cstr_fromformat("%s/zenmusic.kvl", libpath, NULL);

  LOG("writing db to %s", dbpath);

  int res = kvlist_write(dbpath, db.data);
  REL(dbpath);

  if (res < 0) LOG("ERROR db_write cannot write database %s\n", dbpath);

  LOG("db written");
}

void db_gen_genres()
{
  int ei, gi; // entry, genre index

  vec_reset(db.genres);

  for (ei = 0;
       ei < db.songs->length;
       ei++)
  {
    map_t* entry = db.songs->data[ei];
    char*  genre = MGET(entry, "genre");

    if (genre)
    {
      char found = 0;
      for (gi = 0; gi < db.genres->length; gi++)
      {
        char* act_genre = db.genres->data[gi];
        if (strcmp(genre, act_genre) == 0)
        {
          found = 1;
          break;
        }
      }
      if (!found) VADD(db.genres, genre);
    }
  }
}

void db_gen_artists()
{
  int ei;

  map_t* artists = MNEW();

  for (ei = 0;
       ei < db.artists->length;
       ei++)
  {
    map_t* entry  = db.songs->data[ei];
    char*  artist = MGET(entry, "artist");

    if (artist) MPUT(artists, artist, artist);
  }

  map_values(artists, db.artists);

  REL(artists);
}

int db_comp_artist(void* left, void* right)
{
  map_t* l = left;
  map_t* r = right;

  char* la = MGET(l, db.sort_field);
  char* ra = MGET(r, db.sort_field);

  return strcmp(la, ra);
}

void db_filter(char* text)
{
  int ei, vi; // entry, value index

  vec_reset(db.songs);

  vec_reset(db.tmp1);
  vec_reset(db.tmp2);

  map_values(db.data, db.tmp1);

  for (ei = 0;
       ei < db.tmp1->length;
       ei++)
  {
    map_t* entry = db.tmp1->data[ei];
    vec_reset(db.tmp2);
    map_values(entry, db.tmp2);

    for (vi = 0;
         vi < db.tmp2->length;
         vi++)
    {
      char* val = db.tmp2->data[vi];
      if (strstr(val, text))
      {
        vec_add(db.songs, entry);
        break;
      }
    }
  }

  db_gen_genres();
  db_gen_artists();

  vec_sort(db.songs, db_comp_artist);
}

void db_sort(char* field)
{
  if (field != NULL)
  {
    db.sort_field = field;

    vec_reset(db.songs);
    map_values(db.data, db.songs);

    vec_sort(db.songs, db_comp_artist);

    db_gen_genres();
    db_gen_artists();
  }
}

void db_add_entry(char* path, map_t* entry)
{
  MPUT(db.data, path, entry);
  VADD(db.songs, entry); // add immediately, needed on lib analyze to show partial results
}

#endif
