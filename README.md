Zen Multimedia Desktop System

inspired by the golden-era itunes and the tracker module players of the early 90's

slick, lag-free 60 fps smooth and momentum scrolling
retro-minimalistic ui
always visible frequency and scope analyzers
status line and activity window for full transparency
optional library auto-organizing

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

relative paths in library
spectrum, bar analyzer, left/right effect button, unshift bmp?
button on/off alpha change on icon
socketes taviranyitas - 1 byteokkal
accept input on enter
popup-ok center align
go to source/ go to youtube channel button home-ba
fix leaks
fix while heaven wept


ideas

make instead of gmake, debug, release targets -g -O3, install ffmpeg and sdl2 if needed
save small changes to a delta database, add it to the big one at startup
amit releaselni kell, csinalodjon new-val!
test texture resize and reset, texture overflow and reset
add text/input selectors to css, use text style from css
speed up font rendering by using glyph indexes instead of codepoints
grid-based warping of video/album cover based on frequence ( bass in the center )
andromeda: monolith-like particle visu
switch between multiple glyph views in paragraph/one multiglyph view in paragraph  
sanitize css - textfields came class, etc
generic input popup, choice popup view-k
remove non-standard css and html (type=button, blocks)
unit teszter gl_compositorhoz
analog mutatos output meterek is visualizacioban
glossy effekt
scroll-ra lehessen novelni/csokkenteni visuals meretet
statisztikak - top 10 leghallgatottabb, milyen genre-t hallgatsz foleg elmult 1 honap, stb
cortex as visualizer - level gen based on freq, LEFT/DOWN!!!
glFragaCoord? glScissor optimized render
volume fade in/out, cover arton is
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

--

how to be a pro?
- create a project without an ide - learn how compiling, linking, makefile works
 - first command line
  - then with script
  - then makefile
- never leave a result unexamined, an argument unasserted
- learn clojure. your algorithmic skills in any language will be 10x better