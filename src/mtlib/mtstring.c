//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtstr_h
#define mtstr_h

#include "mtmap.c"
#include "mtvector.c"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _str_t str_t;
struct _str_t
{
  uint32_t  length;       // current length of codepoint array
  uint32_t  length_real;  // backing length of codepoint array
  uint32_t  length_bytes; // needed length of byte array for all codepoints
  uint32_t* codepoints;
};

str_t*   str_alloc(void);
void     str_dealloc(void* string);
void     str_reset(str_t* string);
str_t*   str_fromformat(char* format, ...);
str_t*   str_frombytes(char* bytes);
str_t*   str_fromstring(str_t* string);
str_t*   str_substring(str_t* string, int start, int end);
str_t*   str_replace(str_t* string, str_t* newstring, int start, int end);
str_t*   str_compactemojis(str_t* string);
void     str_addstring(str_t* stra, str_t* strb);
void     str_addbytearray(str_t* string, char* bytearray);
void     str_addcodepoint(str_t* string, uint32_t codepoint);
void     str_removecodepointatindex(str_t* string, uint32_t index);
void     str_removecodepointsinrange(str_t* string, uint32_t start, uint32_t end);
void     str_replacecodepoints(str_t* string, uint32_t oldcp, uint32_t newcp);
int8_t   str_compare(str_t* stra, str_t* strb);
int      str_intvalue(str_t* string);
float    str_floatvalue(str_t* string);
uint32_t str_unsignedvalue(str_t* string);
vec_t*   str_split(str_t* string, char character);
map_t*   str_tokenize(str_t* descriptor);
uint32_t str_find(str_t* string, str_t* substring, uint32_t from);
char*    str_bytes(str_t* string);
void     str_describe(void* p, int level);

#endif
#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"
#include <string.h>

#define UTF8_BOM "\xEF\xBB\xBF"

/* creates string */

str_t* str_alloc()
{
  str_t* string = mem_calloc(sizeof(str_t), "str_t", str_dealloc, str_describe);

  string->length       = 0;  // current length of codepoint array
  string->length_real  = 10; // backing length of codepoint array
  string->length_bytes = 0;  // needed length of byte array for all codepoints
  string->codepoints   = mem_calloc(string->length_real * sizeof(uint32_t), "uint32_t*", NULL, NULL);

  return string;
}

/* deletes string */

void str_dealloc(void* pointer)
{
  str_t* string = pointer;
  mem_release(string->codepoints);
}

/* resets string */

void str_reset(str_t* string)
{
  string->length       = 0;
  string->length_real  = 10;
  string->length_bytes = 0;
  string->codepoints   = mem_realloc(string->codepoints, string->length_real * sizeof(uint32_t));
  memset(string->codepoints, 0, string->length_real * sizeof(uint32_t));
}

/* creates string from utf8 bytearray */

str_t* str_fromformat(char* format, ...)
{
  va_list ap;
  char*   text;
  size_t  length = strlen(format);

  va_start(ap, format);
  for (text = format; text != NULL; text = va_arg(ap, char*))
    length += strlen(text);
  length += 1;
  va_end(ap);

  char* result = mem_calloc(sizeof(char) * length, "char*", NULL, NULL);
  va_start(ap, format);
  vsnprintf(result, length, format, ap);
  va_end(ap);

  str_t* resstring = str_frombytes(result);
  mem_release(result);

  return resstring;
}

/* creates string from utf8 bytearray */

str_t* str_frombytes(char* bytes)
{
  if (bytes != NULL)
  {
    str_t* string = str_alloc();
    str_addbytearray(string, bytes);
    return string;
  }
  else
    return NULL;
}

/* creates string from string */

str_t* str_fromstring(str_t* string)
{
  str_t* result = str_alloc();
  str_addstring(result, string);
  return result;
}

/* returns substring of string */

str_t* str_substring(str_t* string, int start, int end)
{
  str_t* result = str_alloc();

  for (int index = start; index < end; index++)
  {
    str_addcodepoint(result, string->codepoints[index]);
  }

  return result;
}

/* replaces substring in string */

str_t* str_replace(str_t* string, str_t* newstring, int start, int end)
{
  str_t* part1 = str_substring(string, 0, start);
  str_t* part2 = str_substring(string, end, string->length);

  str_addstring(part1, newstring);
  str_addstring(part1, part2);

  return part1;
}

/* compact :) and :( to their unicode codepoints */

str_t* str_compactemojis(str_t* string)
{
  str_t* result = str_alloc();

  for (int index = 0; index < string->length; index++)
  {
    if (index < string->length - 1)
    {
      if (string->codepoints[index] == ':' && string->codepoints[index + 1] == ')')
      {
        str_addcodepoint(result, 0x1F601);
        index++;
      }
      else if (string->codepoints[index] == ':' && string->codepoints[index + 1] == '(')
      {
        str_addcodepoint(result, 0x1F61E);
        index++;
      }
      else
        str_addcodepoint(result, string->codepoints[index]);
    }
    else
      str_addcodepoint(result, string->codepoints[index]);
  }
  return result;
}

/* adds string to string */

void str_addstring(str_t* stra, str_t* strb)
{
  if (strb != NULL)
  {
    uint32_t newlength       = stra->length + strb->length;
    uint32_t newlength_real  = stra->length_real + strb->length_real;
    uint32_t newlength_bytes = stra->length_bytes + strb->length_bytes;

    stra->codepoints = mem_realloc(stra->codepoints, sizeof(uint32_t) * newlength_real);
    memcpy((void*)(stra->codepoints + stra->length), (void*)strb->codepoints, strb->length * sizeof(uint32_t));

    stra->length       = newlength;
    stra->length_real  = newlength_real;
    stra->length_bytes = newlength_bytes;
  }
}

/* returns byte count for codepoint */

uint8_t strgetcodebytelength(uint32_t codepoint)
{
  uint8_t codelength = 4;
  if (codepoint < 0x80)
    codelength = 1;
  else if (codepoint < 0x800)
    codelength = 2;
  else if (codepoint < 0x1000)
    codelength = 3;
  return codelength;
}

/* adds buffer containing bytes for one codepoint */

void str_addbuffer(str_t* string, char* buffer, char length)
{
  // filter byte order mark
  if (strcmp(buffer, UTF8_BOM) != 0)
  {
    uint32_t codepoint = 0;
    // extract codepoint
    if (length == 1)
      codepoint = buffer[0] & 0x7F;
    else if (length == 2)
      codepoint = (buffer[0] & 0x1F) << 6 | (buffer[1] & 0x3F);
    else if (length == 3)
      codepoint = (buffer[0] & 0xF) << 12 | (buffer[1] & 0x3F) << 6 | (buffer[2] & 0x3F);
    else if (length == 4)
      codepoint = (buffer[0] & 0x7) << 18 | (buffer[1] & 0x3F) << 12 | (buffer[2] & 0x3F) << 6 | (buffer[3] & 0x3F);
    // add codepoint
    str_addcodepoint(string, codepoint);
  }
}

/* adds utf8 encoded byte array */

void str_addbytearray(str_t* string, char* bytearray)
{
  char buffer[4]       = {0};
  char buffer_position = 0;
  while (*bytearray != 0)
  {
    // checking unicode closing characters or last character
    if ((*bytearray & 0xFF) >> 7 == 0 || (*bytearray & 0xFF) >> 6 == 3)
    {
      if (buffer_position > 0)
      {
        str_addbuffer(string, buffer, buffer_position);
        // reset unicode buffer
        memset(&buffer, 0, 4);
        buffer_position = 0;
      }
    }
    // storing actual byte in unicode codepoint buffer
    buffer[buffer_position++] = *bytearray;
    // step in byte array
    bytearray += 1;
    // invalid utf sequence, aborting
    if (buffer_position == 5) return;
  }
  // add remaining buffer content
  if (buffer_position > 0) str_addbuffer(string, buffer, buffer_position);
}

/* adds code point */

void str_addcodepoint(str_t* string, uint32_t codepoint)
{
  uint8_t codelength = strgetcodebytelength(codepoint);

  // expand
  if (string->length_real == string->length)
  {
    string->codepoints = mem_realloc(string->codepoints, sizeof(uint32_t) * (string->length_real + 10));
    string->length_real += 10;
  }

  string->codepoints[string->length] = codepoint;
  string->length += 1;
  string->length_bytes += codelength;
}

/* removes codepoint */

void str_removecodepointatindex(str_t* string, uint32_t index)
{
  uint32_t codepoint  = string->codepoints[index];
  uint8_t  codelength = strgetcodebytelength(codepoint);

  string->length_bytes -= codelength;
  memmove(string->codepoints + index, string->codepoints + index + 1, (string->length - index - 1) * sizeof(uint32_t));
  string->length -= 1;
}

/* removes codepoints in range */

void str_removecodepointsinrange(str_t* string, uint32_t start, uint32_t end)
{
  if (end > string->length) end = string->length;

  for (int index = start; index < end; index++)
  {
    uint32_t codepoint  = string->codepoints[index];
    uint8_t  codelength = strgetcodebytelength(codepoint);
    string->length_bytes -= codelength;
  }

  if (end < string->length)
  {
    memmove(
        string->codepoints + start,
        string->codepoints + end + 1,
        (string->length - end - 1) * sizeof(uint32_t));
  }

  string->length -= end - start + 1;
}

/* replaces codepoints */

void str_replacecodepoints(str_t* string, uint32_t oldcp, uint32_t newcp)
{
  for (int index = 0; index < string->length; index++)
  {
    if (string->codepoints[index] == oldcp) string->codepoints[index] = newcp;
  }
}

/* compares two string */

int8_t str_compare(str_t* stra, str_t* strb)
{
  char* bytes_a = str_bytes(stra);
  char* bytes_b = str_bytes(strb);

  int8_t result = strcmp(bytes_a, bytes_b);

  mem_release(bytes_a);
  mem_release(bytes_b);

  return result;
}

/* returns intvalue */

int str_intvalue(str_t* string)
{
  char* viewindexc = str_bytes(string);
  int   viewindex  = atoi(viewindexc);
  mem_release(viewindexc);
  return viewindex;
}

/* returns floatvalue */

float str_floatvalue(str_t* string)
{
  char* viewindexc = str_bytes(string);
  float viewindex  = atof(viewindexc);
  mem_release(viewindexc);
  return viewindex;
}

/* returns unsigned value */

uint32_t str_unsignedvalue(str_t* string)
{
  char*         valuec = str_bytes(string);
  unsigned long value  = strtoul(valuec, NULL, 0);
  mem_release(valuec);
  return (uint32_t)value;
}

/* splits string at codepoint to a vector */

vec_t* str_split(str_t* string, char codepoint)
{
  vec_t* vector  = vec_alloc();
  str_t* segment = str_alloc();
  for (int index = 0; index < string->length; index++)
  {
    if (string->codepoints[index] == codepoint)
    {
      // add word to result, create new word
      if (segment->length > 0)
      {
        vec_add(vector, segment);
        mem_release(segment);
        segment = str_alloc();
      }
    }
    else
      str_addcodepoint(segment, string->codepoints[index]);
  }
  // add word to result
  if (segment->length > 0) vec_add(vector, segment);
  mem_release(segment);
  return vector;
}

/* splits string at spaces and creates key-value pairs */

map_t* str_tokenize(str_t* descriptor)
{
  map_t* map    = map_alloc();
  vec_t* tokens = str_split(descriptor, ' ');
  for (int index = 0; index < tokens->length; index += 2)
  {
    char* key = str_bytes(tokens->data[index]);
    map_put(map, key, tokens->data[index + 1]);
    mem_release(key);
  }
  mem_release(tokens);
  return map;
}

/* finds substring in string from given index */

uint32_t str_find(str_t* string, str_t* substring, uint32_t from)
{
  if (string == NULL) return UINT32_MAX;
  if (substring == NULL) return UINT32_MAX;
  if (string->length < substring->length) return UINT32_MAX;

  for (uint32_t index = from; index < string->length - substring->length + 1; index++)
  {
    if (string->codepoints[index] == substring->codepoints[0])
    {
      uint32_t count;
      for (count = 1; count < substring->length; count++)
      {
        if (string->codepoints[index + count] != substring->codepoints[count]) break;
      }
      if (count == substring->length) return index;
    }
  }
  return UINT32_MAX;
}

/* returns backing bytearray */

char* str_bytes(str_t* string)
{
  if (string == NULL) return NULL;
  char*    bytes    = mem_calloc((string->length_bytes + 1) * sizeof(char), "char*", NULL, NULL);
  uint32_t position = 0;
  for (int index = 0; index < string->length; index++)
  {
    uint32_t codepoint = string->codepoints[index];
    if (codepoint < 0x80)
    {
      bytes[position++] = codepoint;
    }
    else if (codepoint < 0x800)
    {
      bytes[position++] = (codepoint >> 6) | 0xC0;
      bytes[position++] = (codepoint & 0x3F) | 0x80;
    }
    else if (codepoint < 0x1000)
    {
      bytes[position++] = (codepoint >> 12) | 0xE0;
      bytes[position++] = ((codepoint >> 6) & 0x3F) | 0x80;
      bytes[position++] = (codepoint & 0x3F) | 0x80;
    }
    else
    {
      bytes[position++] = (codepoint >> 18) | 0xF0;
      bytes[position++] = ((codepoint >> 12) & 0x3F) | 0x80;
      bytes[position++] = ((codepoint >> 6) & 0x3F) | 0x80;
      bytes[position++] = (codepoint & 0x3F) | 0x80;
    }
  }
  return bytes;
}

void str_describe(void* p, int level)
{
  str_t* str = p;
  printf("length %u", str->length);
}

#endif
