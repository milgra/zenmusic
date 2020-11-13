#ifndef mtparser_h
#define mtparser_h

#include "mtmap.c"
#include <stdio.h>
#include <stdlib.h>

mtmap_t* parse_html(char* path);
mtmap_t* parse_css(char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

char* parse_read(char* path)
{
  FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* string = malloc(fsize + 1);
  fread(string, 1, fsize, f);
  fclose(f);

  string[fsize] = 0;
}

mtmap_t* parse_html(char* path)
{
  // <div -> create new element, increase level
  // /> </div> -> close element, decrease level
  // id= -> set view name, check for style
  // class= -> check for style
}

mtmap_t* parse_css(char* path)
{
  // #xy, .xy -> create rule
  // } -> end rule
  // background-color -> set background color
}

#endif
