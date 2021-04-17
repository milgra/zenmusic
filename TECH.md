# Zen Music music player and organizer technical information #

1. Structure
2. Development
3. Contribution

## 1. Structure ##


robust database handling
zmusdb, text based key-value pairs
no modification, zmusdb_new created, after succesfull backup it is renamed to zmusdb
small changes are stored in zmusdb_upd, merged after start

zmusdb is a key-value-list kvl file

## 2. Development ##

pkg install sdl2 ffmpeg opengl gmake clang-format
hook clang-format and gmake to file save hook

## 3. Contribution ##

Please follow these guidelines :

- use clang format before commiting/after file save
- use mtlib (memory,vector,map) for memory handling
- create a new test for any new feature you add
- always run all tests before push
- always run valgrind and check for leaks before push
