Zen Multimedia Desktop System

Main goals : operating transparency - all events are logged in event viewer

zen media - offline file/media viewer and manager / preview area, file info area
zen music - offline music player and manager / video/album area, two spectrum/freq analyzer for stereo channels
zen video - offline photo/video viewer and library manager / video/image area, location area, info area

development :

pkg install sdl2 ffmpeg opengl gmake clang-format
hook clang-format and gmake to file save hook

---

working vol/seek ring, play/pause, mute/unmute
knob folotti scroll tekerje a knobot
search/filter bar - genre is metal, name contains x, year between x and y
scrollbar
text anim
database
uj event villanjon fel artist helyen, odakattintva event viewer, 
tilemap is auto-resize legyen duplazassal
volume bar pedig play button korul legyen egy gyuru - kell arc mtdrawer-be
search bar click hozza elo genre/artist listat
resize textures on window change if needed
unit teszter gl_compositorhoz
ha az egeret a lista item jobb oldalara viszed popupban mutassa id3/song infokat es album artot
database, reordering
resizable header/songlist border
check leaks
bar analyzer
unshift bmp - spectrum analyzer shift left
grid-based warping of video/album cover based on frequence ( bass in the center )
ujdonsagok video jojjon fel az events windowban, about
zold/piros listitem background play/pausre, animalt!!!
analog mutatos output meterek is visualizacioban
shuffle ikon egy dobokocka legyen, hover-re animaljon 3-6-2
repeat ikon egy gyurus nyil legyen
glossy effekt
scroll-ra lehessen novelni/csokkenteni visuals meretet
statisztikak - top 10 leghallgatottabb, milyen genre-t hallgatsz foleg elmult 1 honap, stb
cortex as visualizer - level gen based on freq, LEFT/DOWN!!!
glFragaCoord ?
volume fade in/out, cover arton is
donate button /donate popup after 100 startups

zen media :
space start media

optimize :
only update vertexes when frame change
only update shadow/blur when frame change
only update visu bitmaps when needed