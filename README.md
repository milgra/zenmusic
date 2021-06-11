# Zen Music music player, organizer and visualizer
"touch your music"

![alt text](svg/screenshot.jpeg)

Zen Music is inspired by the 2000's era iTunes and the tracker music players of the early 90's.

## Features ##

- standalone UI renderer with slick, lag-free 60 fps smooth and momentum scrolling
- retro-minimalistic UI
- always visible frequency and scope analyzer overlays
- status line and activity window for full transparency
- library auto-organizing ( if enabled )
- human readable text-based database for transparency & easy modification

Video introduction


## Installation ##

### FreeBSD ###

```
git clone https://github.com/milgra/zenmusic.git
cd zenmusic
make install
```

### Linux ###

```
git clone https://github.com/milgra/zenmusic.git
cd zenmusic
```

modify include paths in makefile to use your distribution's path

```
make install
```

### Windows ###

download latest build from here :  
unzip  
start zenmusic exe from extracted folder  


## User Guide ##

[Open User Guide](USER.md)


## Feedback ##

Please report issues and add feature requests here on github.


## Libraries used ##

- FFMPEG / media parsing
- SDL2 / window/graphics context handling
- stb_truetype / text generation


## Programs used ##

- Inkscape for the icons
- GNU Emacs for programming
- FreeBSD for development platform


## Contribute ##

Contributors are welcome!

Technical video introduction

[Tech Guide](USER.md)