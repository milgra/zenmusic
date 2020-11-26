//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtcstr_h
#define mtcstr_h

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

char*    mtcstr_fromformat(char* format, ...);
char*    mtcstr_fromcstring(char* string);
char*    mtcstr_fromfile(char* path);
uint32_t mtcstr_color_from_cstring(char* string);
char*    mtcstr_generate_readablec(uint32_t length);
char*    mtcstr_generate_alphanumeric(uint32_t length);
void     mtcstr_describe(void* p, int level);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"
#include <string.h>

static char hexa[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      // 0..9
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      // 10..19
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      // 20..29
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      // 30..39
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1,      // 40..49
        2, 3, 4, 5, 6, 7, 8, 9, 0, 0,      // 50..59
        0, 0, 0, 0, 0, 10, 11, 12, 13, 14, // 60..69
        15, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 70..79
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      // 80..89
        0, 0, 0, 0, 0, 0, 0, 10, 11, 12,   // 90..99
        13, 14, 15, 0, 0, 0, 0, 0, 0, 0    // 100..109
};

/* returns uint value based on digits */

uint32_t mtcstr_color_from_cstring(char* string)
{
  uint32_t result = 0;
  while (*string != 0)
    result = result << 4 | hexa[*string++];
  return result;
}

/* creates string from utf8 bytearray */
/* PARAMETER LIST MUST END WITH NULL!!! */

char* mtcstr_fromformat(char* format, ...)
{
  va_list ap;
  char*   text;
  size_t  length = strlen(format);

  va_start(ap, format);
  for (text = format; text != NULL; text = va_arg(ap, char*))
    length += strlen(text);
  length += 1;
  va_end(ap);

  char* result = mtmem_calloc(sizeof(char) * length, "char*", NULL, mtcstr_describe);
  va_start(ap, format);
  vsnprintf(result, length, format, ap);
  va_end(ap);

  return result;
}

/* copies c string with managed memory space */

char* mtcstr_fromcstring(char* string)
{
  char* result = NULL;
  if (string != NULL)
  {
    result = mtmem_calloc((strlen(string) + 1) * sizeof(char), "char*", NULL, mtcstr_describe);
    memcpy(result, string, strlen(string));
  }
  return result;
}

/* reads up text file */

char* mtcstr_fromfile(char* path)
{

  char* buffer = NULL;
  int   string_size, read_size;
  FILE* handler = fopen(path, "r");

  if (handler)
  {
    // Seek the last byte of the file
    fseek(handler, 0, SEEK_END);
    // Offset from the first to the last byte, or in other words, filesize
    string_size = ftell(handler);
    // go back to the start of the file
    rewind(handler);

    // Allocate a string that can hold it all
    buffer = (char*)mtmem_calloc(sizeof(char) * (string_size + 1), "char*", NULL, NULL);

    // Read it all in one operation
    read_size = fread(buffer, sizeof(char), string_size, handler);

    // fread doesn't set it so put a \0 in the last position
    // and buffer is now officially a string
    buffer[string_size] = '\0';

    if (string_size != read_size)
    {
      // Something went wrong, throw away the memory and set
      // the buffer to NULL
      free(buffer);
      buffer = NULL;
    }

    // Always remember to close the file.
    fclose(handler);
  }

  return buffer;
}

/* generates readable string */

char* vowels     = "aeiou";
char* consonants = "bcdefghijklmnpqrstvwxyz";

char* mtcstr_generate_readablec(uint32_t length)
{
  char* result = mtmem_calloc(sizeof(char) * (length + 1), "char*", NULL, mtcstr_describe);
  for (int index = 0; index < length; index += 2)
  {
    result[index] = consonants[rand() % strlen(consonants)];
    if (index + 1 < length) result[index + 1] = vowels[rand() % strlen(vowels)];
  }
  return result;
}

/* generates alphanumeric string */

char* mtcstr_alphanumeric =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

char* mtcstr_generate_alphanumeric(uint32_t length)
{
  char* result = mtmem_calloc(sizeof(char) * (length + 1), "char*", NULL, mtcstr_describe);
  for (int index = 0; index < length; index++)
  {
    result[index] = mtcstr_alphanumeric[rand() % strlen(mtcstr_alphanumeric)];
  }
  return result;
}

void mtcstr_describe(void* p, int level)
{
  printf("%s", (char*)p);
}

#endif
