Zen Multimedia Desktop System

Main goals : operating transparency - all events are logged in event viewer

zen media - offline file/media viewer and manager / preview area, file info area
zen music - offline music player manager and visualizer / video/album area, two spectrum/freq analyzer for stereo channels
zen video - offline photo/video viewer and library manager / video/image area, location area, info area

development :

pkg install sdl2 ffmpeg opengl gmake clang-format
hook clang-format and gmake to file save hook

---

first version

button on/off alpha change on icon
display font rendering fix, center
artists/genres button (A, G karika ket oldalon? )
additional columns - genre, last played, added, skipped, play count,
fix leaks
elso indulasnal analyzer progresst mutassa valahol amig zenet hallgatsz
unicode support files, metadata!!!
tilemap is auto-resize legyen duplazassal
zold/piros listitem background play/pausre, animalt!!!
scrollbar drag
with the help of C & EMACS 
ui color shade selector
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

search bar csusszon ki header alol ha songlistheaderre megy az eger
id3 editor
unit teszter gl_compositorhoz
ha az egeret a lista item jobb oldalara viszed popupban mutassa id3/song infokat es album artot
resizable header/songlist border
grid-based warping of video/album cover based on frequence ( bass in the center )

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