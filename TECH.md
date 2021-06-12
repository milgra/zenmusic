# Zen Music technical information
for contributors and developers

1. Overview

Zen Music is a pure C project written in [headerless C](https://github.com/milgra/headerlessc).
It uses the ffmpeg library for media decoding/encoding/transcoding, SDL2 library for window and audio handling and OpenGL context creation.
It uses a custom UI renderer called Zen UI, it is backed by OpenGL at the moment, Vulkan backend is on the roadmap.
It uses the Zen Core library for memory management, map/vector/bitmap container implementations, utf8 string and math functions.

2. Program structure

Graphics stack :

[OS][OPENGL] -> [SDL2] -> [ZEN_WM] -> [ZEN UI] -> ui.c -> ui controllers

Media stack :

[OS] -> [SDL2][ffmpeg] -> [ZEN MEDIA PLAYER][ZEN MEDIA TRANSCODER] -> zenmusic.c




1. Structure  
2. Development  
3. Contribution  

## 1. Structure ##

robust database handling
zmusdb, text based key-value pairs
no modification, zmusdb_new created, after succesfull backup it is renamed to zmusdb
small changes are stored in zmusdb_upd, merged after start

zmusdb is a key-value-list kvl file

ui -
gl-connector layer - the higher layers sending triangles to it with texture ids, renders these triangles with textures
ui_compositor - collects view rectangles, puts view textures into texture maps, sends textures and vertexes to gl_connector layer
ui_generator - collects views, sets up texture channels, renders view bitmaps in background thread
ui_manager - it has the root view for the ui, resends views to generator when add/remove happened

modules - components isolated from each other
mtlib - core library for memory management, containers
ui - ui handlers/components

## 2. Development ##

```
pkg install sdl2 ffmpeg opengl gmake clang-format
```

hook clang-format and gmake to file save hook


## 3. Contribution ##

Please follow these guidelines :

- use clang format before commiting/after file save
- use mtlib (memory,vector,map) for memory handling ( should modify mtcstring_fromformat to avoid NULL usage )
- create a new test for any new feature you add
- if you mofify existing code be sure that a test covers your modification
- always run all tests before push
- always run valgrind and check for leaks before push