Zen Multimedia Desktop System

Main goals : operating transparency - all events are logged in event viewer

zen media - offline file/media viewer and manager / preview area, file info area
zen music - offline music player manager and visualizer / video/album area, two spectrum/freq analyzer for stereo channels
zen image - offline photo/video viewer and library manager / video/image area, location area, info area

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

highlight row green/red
listitem hidden legyen 
dfdmessages window, log messages
move songs to folders
additional columns - genre, last played, added, skipped, play count,
fix leaks
genre/tags list, artist list
id editor window
elso indulasnal analyzer progresst mutassa valahol amig zenet hallgatsz
unicode support files, metadata!!!
tilemap is auto-resize legyen duplazassal
scrollbar drag
jobb halfra vive az egeret song folott id3 editor

---

next version

search/filter bar - genre is metal, name contains x, year between x and y
uj event villanjon fel artist helyen, odakattintva event viewer, 
spectrum, bar analyzer, left/right effect button, unshift bmp?
config file - library path, visu state,
speed up font rendering by using glyph indexes instead of codepoints

---

future features

ui color shade selector, darknal invertalja a jelenlegi semat
button on/off alpha change on icon
search bar csusszon ki header alol ha songlistheaderre megy az eger
id3 editor
unit teszter gl_compositorhoz
ha az egeret a lista item jobb oldalara viszed popupban mutassa id3/song infokat es album artot
resizable header/songlist border
grid-based warping of video/album cover based on frequence ( bass in the center )
andromeda:  monolith-like particle visu
swithc between multiple glyph views in paragraph/one multiglyph view in paragraph  

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