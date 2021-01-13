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

id editor, album url, album preview
fix query field
make genre/artist selection filter songs, fill up query bar
increase play count
fix leaks
elso indulasnal analyzer progresst mutassa valahol amig zenet hallgatsz
scrollbar drag
config file - default library
text overflow fix
save small changes to a delta database, add it to the big one at startup
zenmusic /usr/milgra/Testmusic - select lib folder
refresh database gomb legyen az egyik, settings, about a masik ketto
amit releaselni kell, csinalodjon new-val!
get extension from stream for organizing
relative paths in library
track number in song name for organizing
test texture resize and reset, texture overflow and reset
add text/input selectors to css, use text style from css
search/filter bar - genre is metal, name contains x, year between x and y
uj event villanjon fel artist helyen, odakattintva event viewer, 
spectrum, bar analyzer, left/right effect button, unshift bmp?
speed up font rendering by using glyph indexes instead of codepoints
ui color shade selector, darknal invertalja a jelenlegi semat
button on/off alpha change on icon
unit teszter gl_compositorhoz
grid-based warping of video/album cover based on frequence ( bass in the center )
andromeda:  monolith-like particle visu
swithc between multiple glyph views in paragraph/one multiglyph view in paragraph  
debug button - show texmap in settings

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