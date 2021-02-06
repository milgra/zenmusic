#ifndef db_h
#define db_h

#include "mtmap.c"

void db_init();
void db_read(char* libpath);
void db_write(char* libpath);
void db_filter(char* text);
void db_add_entry(char* path, map_t* entry);
void db_sort(char* field, int toggle);

map_t*   db_get_db();
vec_t*   db_get_songs();
vec_t*   db_get_genres();
vec_t*   db_get_artists();
uint32_t db_count();
void     db_reset();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "mtcstring.c"
#include "mtlog.c"
#include "mtvector.c"
#include <ctype.h>

struct _db_t
{
  map_t* data;
  vec_t* songs;
  vec_t* genres;
  vec_t* artists;

  char*   sort_field;
  vsdir_t sort_dir;

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

void db_reset()
{
  map_reset(db.data);
  vec_reset(db.songs);
  vec_reset(db.genres);
  vec_reset(db.artists);
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

int db_comp_entry(void* left, void* right)
{
  map_t* l = left;
  map_t* r = right;

  char* la = MGET(l, db.sort_field);
  char* ra = MGET(r, db.sort_field);

  return strcmp(la, ra);
}

int db_comp_text(void* left, void* right)
{
  char* la = left;
  char* ra = right;

  return strcmp(la, ra);
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

  vec_reset(db.artists);

  map_t* artists = MNEW();

  for (ei = 0;
       ei < db.songs->length;
       ei++)
  {
    map_t* entry  = db.songs->data[ei];
    char*  artist = MGET(entry, "artist");

    if (artist) MPUT(artists, artist, artist);
  }

  map_values(artists, db.artists);
  vec_sort(db.artists, VSD_ASC, db_comp_text);

  REL(artists);
}

int db_nextword(char* text, char* part)
{
  if (strlen(part) < strlen(text))
  {
    for (int i = 0; i < strlen(part); i++)
    {
      if (text[i] != part[i]) return 0;
    }
    return 1;
  }
  return 0;
}

map_t* db_query_fields(char* text)
{
  if (strstr(text, " "))
  {
    map_t* fields = MNEW();
    char*  key    = NULL;
    char*  val    = NULL;
    int    last   = 0;
    int    i      = 0;

    for (i = 0; i < strlen(text); i++)
    {
      if (db_nextword(text + i, " is") && key == NULL)
      {
        key = mem_calloc(i - last + 1, "char", NULL, NULL);
        memcpy(key, text + last, i - last);
        i += 4;
        last = i;
        printf("key %s\n", key);
      }

      if (db_nextword(text + i, " and") && val == NULL)
      {
        val = mem_calloc(i - last + 1, "char", NULL, NULL);
        memcpy(val, text + last, i - last);
        i += 5;
        last = i;
        printf("val %s\n", val);

        cstr_tolower(key);
        MPUT(fields, key, val);
        REL(key);
        REL(val);
        key = NULL;
        val = NULL;
      }
    }
    if (last < i && val == NULL && key != NULL)
    {
      val = mem_calloc(i - last, "char", NULL, NULL);
      memcpy(val, text + last, i - last);
      printf("val %s\n", val);

      cstr_tolower(key);
      MPUT(fields, key, val);
      REL(key);
      REL(val);
      key = NULL;
      val = NULL;
    }

    return fields;
  }
  return NULL;
}

void db_filter(char* text)
{
  int ei, ki; // entry, key index

  map_t* fields = db_query_fields(text);
  char*  value  = NULL;
  char*  query  = NULL;

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

    if (fields)
    {
      // use query fields
      map_keys(fields, db.tmp2);
    }
    else
    {
      // use all fields
      map_keys(entry, db.tmp2);
    }

    for (ki = 0;
         ki < db.tmp2->length;
         ki++)
    {
      char* field = db.tmp2->data[ki];
      value       = MGET(entry, field);

      if (value)
      {
        if (fields)
          query = MGET(fields, field);
        else
          query = text;

        if (strstr(value, query))
        {
          vec_add(db.songs, entry);
          break;
        }
      }
      else
      {
        printf("NO %s field in entry :\n", field);
        mem_describe(entry, 0);
        printf("\n");
      }
    }
  }

  if (fields) REL(fields);

  db_gen_genres();
  db_gen_artists();

  db.sort_field = "artist";
  vec_sort(db.songs, db.sort_dir, db_comp_entry);
}

void db_sort(char* field, int toggle)
{
  if (field != NULL)
  {
    if (toggle == 1 && db.sort_field != NULL && strcmp(field, db.sort_field) == 0)
    {
      if (db.sort_dir == VSD_ASC)
        db.sort_dir = VSD_DSC;
      else
        db.sort_dir = VSD_ASC;
    }
    else
    {
      db.sort_dir = VSD_ASC;
    }

    db.sort_field = field;

    vec_reset(db.songs);
    map_values(db.data, db.songs);

    vec_sort(db.songs, db.sort_dir, db_comp_entry);

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
