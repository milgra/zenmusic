Zen Multimedia Desktop System

Main goals : operating transparency - all events are logged in event viewer

zen media - offline file/media viewer and manager / preview area, file info area
zen music - offline music player manager and visualizer / video/album area, two spectrum/freq analyzer for stereo channels
zen image - offline photo/video viewer and library manager / video/image area, location area, info area


Zen Music

robust database handling
zmusdb, text based key-value pairs
no modification, zmusdb_new created, after succesfull backup it is renamed to zmusdb
small changes are stored in zmusdb_upd, merged after start

zmusdb is a key-value-list kvl file

development :

pkg install sdl2 ffmpeg opengl gmake clang-format
hook clang-format and gmake to file save hook

how to delete a song : just delete it from the file system, zen music will clear it up from the lib after restart

how to remote control : send 0 for play/pause, 1 for prev song, 2 for next song to localhost:732

libraries used :

FFMPEG / media parsing
SDL2 / window/graphics context handling
stb_truetype / text generation

programs used :

Inkscape for the icons
emacs for programming
freebsd for os

---

first version

settings view - select library, organize library, color selector
close icons to all popups
title/aritst/info egy textfield legyen, ugy konnyebb shadow is, time bar passed/remaining
fix query field
make genre/artist selection filter songs, fill up query bar
increase play count
fix leaks
elso indulasnal analyzer progresst mutassa valahol amig zenet hallgatsz
scrollbar drag
save small changes to a delta database, add it to the big one at startup
amit releaselni kell, csinalodjon new-val!
relative paths in library
track number in song name for organizing
test texture resize and reset, texture overflow and reset
add text/input selectors to css, use text style from css
search/filter bar - genre is metal, name contains x, year between x and y
spectrum, bar analyzer, left/right effect button, unshift bmp?
speed up font rendering by using glyph indexes instead of codepoints
ui color shade selector, darknal invertalja a jelenlegi semat
button on/off alpha change on icon
unit teszter gl_compositorhoz
grid-based warping of video/album cover based on frequence ( bass in the center )
andromeda:  monolith-like particle visu
swithc between multiple glyph views in paragraph/one multiglyph view in paragraph  
kijelzon shadow
time onclick = remaining time
socketes taviranyitas - 1 byteokkal
accept input on enter

ideas

ujdonsagok video jojjon fel az events windowban, about
analog mutatos output meterek is visualizacioban
glossy effekt
scroll-ra lehessen novelni/csokkenteni visuals meretet
statisztikak - top 10 leghallgatottabb, milyen genre-t hallgatsz foleg elmult 1 honap, stb
cortex as visualizer - level gen based on freq, LEFT/DOWN!!!
glFragaCoord? glScissor optimized render
volume fade in/out, cover arton is
donate button /donate popup after 100 startups
resync library button
replace stb image with ffmpeg

zen media :
space start media
info/hex tab
binary files shows up immediately as hex in media viewer
clipboard rect - drag multiple files, preview those files

optimize :
only update vertexes when frame change
only update shadow/blur when frame change
only update visu bitmaps when needed

---

Functional test

// songlist - songlist header

resize header cells
rearrange header cells
sort on header cell select

// songlist - query field

sort on query


---

intro video

Hi, My name is Milan, I\m proiud to present the first version of Zen Music player and manager.
So why did I create another music player?
I wanted the smoothness, beauty and simlicity of macos itunes on unix-like systems and I also wanted always-on freqeuncy and scope analyzers like in tracker module players from the 80's-90's.
So Zen Media is capable of
- retro-minimalistic design optimized for touchpad scrolling 
- 60 fps smooth scrolling of your library
- knob-based volume and seek bar instead of the outdated slider bars
- sub-pixel antialiased glyph rendering & glyph animation
- frequency/scope analyzer
- auto-organizing your lib if wanted
- total transparency with the activity window and human readable database format

tech video

two big parts : ui - glyph, text, view handling, rendering and compositing
media transcoding : media file decoding, playing, metadata and cover art encoding

ui -
gl-connector layer - the higher layers sending triangles to it with texture ids, renders these triangles with textures
ui_compositor - collects view rectangles, puts view textures into texture maps, sends textures and vertexes to gl_connector layer
ui_generator - collects views, sets up texture channels, renders view bitmaps in background thread
ui_manager - it has the root view for the ui, resends views to generator when add/remove happened