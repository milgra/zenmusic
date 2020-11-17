#ifndef html_h
#define html_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _range_t
{
  uint32_t pos;
  uint32_t len;
} range_t;

typedef struct _tag_t
{
  uint32_t pos;
  uint32_t len;

  uint32_t level;
  uint32_t parent;

  range_t id;
  range_t class;
} tag_t;

typedef struct _prop_t
{
  range_t class;
  range_t key;
  range_t value;
} prop_t;

char*   html_read(char* path);
tag_t*  html_parse_html(char* path);
prop_t* html_parse_css(char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

char* html_read(char* path)
{
  FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = mtmem_alloc(fsize + 1, "char*", NULL, mtcstr_describe);

  fread(string, 1, fsize, f);
  fclose(f);

  string[fsize] = 0;

  return string;
}

uint32_t count_tags(char* html)
{
  int   t = 0; // tag index
  char* c = html;
  while (*c)
  {
    if (*c == '<')
      t++;
    c++;
  }
  return t;
}

void extract_tags(char* html, tag_t* tags)
{
  uint32_t t = 0; // tag index
  uint32_t i = 0; // char index
  char*    c = html;
  while (*c)
  {
    if (*c == '<')
      tags[t].pos = i;
    else if (*c == '>')
    {
      tags[t].len = i - tags[t].pos + 1;
      t++;
    }
    i++;
    c++;
  }
}

range_t extract_string(char* str, uint32_t pos, uint32_t len)
{
  int start = 0;
  int end   = 0;
  int in    = 0;
  for (int i = pos; i < pos + len; i++)
  {
    char c = str[i];
    if (c == '"')
    {
      if (!in)
      {
        in    = 1;
        start = i;
      }
      else
      {
        in  = 0;
        end = i;
        break;
      }
    }
  }
  if (!in)
    return ((range_t){.pos = start, .len = end - start - 1});
  else
    return ((range_t){0});
}

range_t extract_value(tag_t tag, char* key, char* html)
{
  char*    start = strstr(html + tag.pos, key);
  uint32_t len   = start - (html + tag.pos);

  if (len < tag.len)
  {
    range_t range = extract_string(html, start - html, tag.len);
    return range;
  }
  return ((range_t){0});
}

void analyze_tags(char* html, tag_t* tags, uint32_t count)
{
  int l = 0; // level
  for (int i = 0; i < count; i++)
  {
    tags[i].level = l++;

    int ii = i;
    while (ii-- > 0)
    {
      if (tags[ii].level == tags[i].level - 1)
      {
        tags[i].parent = ii;
        break;
      }
    }

    tags[i].id    = extract_value(tags[i], "id=\"", html);
    tags[i].class = extract_value(tags[i], "class=\"", html);

    if (html[tags[i].pos + 1] == '/')
      l -= 2; // </div>
    if (html[tags[i].pos + tags[i].len - 2] == '/')
      l -= 1; // />
  }
}

tag_t* html_parse_html(char* html)
{
  uint32_t cnt  = count_tags(html);
  tag_t*   tags = mtmem_calloc(sizeof(tag_t) * (cnt + 1), "tag_t*", NULL, NULL);

  extract_tags(html, tags);
  analyze_tags(html, tags, cnt);

  for (int i = 0; i < cnt; i++)
  {
    tag_t t = tags[i];
    //printf("ind %i tag %.*s lvl %i par %i\n", i, t.len, html + t.pos, t.level, t.parent);
  }

  return tags;
}

uint32_t count_props(char* css)
{
  int   t = 0; // tag index
  char* c = css;
  while (*c)
  {
    if (*c == ':')
      t++;
    c++;
  }
  return t;
}

void analyze_classes(char* css, prop_t* props)
{
  int      start = 0;
  char     in_w  = 0; // in word
  char     in_c  = 0; // in class
  char     in_p  = 0; // in property
  uint32_t index = 0;
  range_t class  = {0};
  char* c        = css;
  while (*c)
  {
    if (!in_w)
    {
      if (*c != ' ' && *c != '\n' && *c != '\r' && *c != '{' && *c != '}')
      {
        start = c - css;
        in_w  = 1;
      }
    }
    else
    {
      if (*c == ' ')
      {
        in_w      = 0;
        class.pos = start;
        class.len = c - css - start;
      }
      else if (*c == ':')
      {
        in_w                 = 0;
        props[index].class   = class;
        props[index].key.pos = start;
        props[index].key.len = c - css - start;
      }
      else if (*c == ';')
      {
        in_w                   = 0;
        props[index].class     = class;
        props[index].value.pos = start;
        props[index].value.len = c - css - start;
        index++;
      }
    }
    c++;
  }
}

prop_t* html_parse_css(char* css)
{
  mtmap_t* map   = mtmap_alloc();
  uint32_t cnt   = count_props(css);
  prop_t*  props = mtmem_calloc(sizeof(prop_t) * (cnt + 1), "prop_t*", NULL, NULL);

  analyze_classes(css, props);

  for (int i = 0; i < cnt; i++)
  {
    prop_t p = props[i];
    //printf("extracted prop %.*s %.*s %.*s\n", p.class.len, css + p.class.pos, p.key.len, css + p.key.pos, p.value.len, css + p.value.pos);
  }

  return props;
}

#endif