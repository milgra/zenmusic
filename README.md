# Zen Music

Zen Music is a music player, organizer and visualizer inspired by the 2000's era iTunes and the tracker music players of the early 90's.

![alt text](svg/screenshot.jpeg)

## Features ##

- retro-minimalistic UI
- beautiful and smooth UX experience
- always-on frequency and scope analyzer visualizers
- library auto-organization ( if enabled )
- activity window and human-readable database for transparent operation

Read the user guide for further information : [Open User Guide](USER.md)

## Installation ##

### FreeBSD ###

```
git clone https://github.com/milgra/zenmusic.git
cd zenmusic
sudo gmake deps
gmake rel
sudo gmake install
```

### Linux ###

install the following packages :

ffmpeg sdl2 glew


```
git clone https://github.com/milgra/zenmusic.git
cd zenmusic
make rel
sudo make install
```

## User Guide ##

[Open User Guide](USER.md)

## Feedback ##

Please report issues and add feature requests here on github.

## Libraries used - Thanks for creating these! ##

- FFMPEG / media parsing
- SDL2 / window/graphics context handling
- stb_truetype / text generation
- Neil Hanning's utf8.h / case-insensitive utf8 comparison

## Programs used - Thanks for creating these! ##

- Inkscape for the icons
- GNU Emacs for programming
- FreeBSD for development platform

## Contribute ##

Contributors are welcome!

## Tech Guide ##

[Tech Guide](TECH.md)

## Roadmap ##

[Roadmap](ROAD.md)
