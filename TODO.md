---

first version

filter background error
filter deactivate
vh_list set active frame, visualizer overlay
filter fix

flag non-editable fields in database, show them as grey in editor
gombok kattanjanak
clear cover art if not available
set starting volume
artist-album sort
search/filter history ( use filters for playlists)
on mute fade in/out volume with alpha
header resize on hscroll problem
socketes taviranyitas - 1 byteokkal
accept input on enter
fix leaks
fix while heaven wept, black lotus
fix mp4 playing freezes
rdft ne logaritmikus legyen, alig latszik a kozep tartomany
text style is cssbol jojjon
metadata update eseten sulyos leak, a hatterben tortenjen, analyzer kozben is lehessen mar bongeszni
dupla klikk szarakszik
automata frame rate keep - ha 50 fps ala megy, rakja backgroundba a rendert
link a youtube videora ami a hasznalatat mutatja be
UI Recorder - UI player API, -uiscript
song list ugorjon az aktualisan jatszott szamra
date added
how to remote control : send 0 for play/pause, 1 for prev song, 2 for next song to localhost:732

ideas

textinput autoscroller
rdft left/right channel
full screen visualizers
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



zen media - offline file/media viewer and manager / preview area, file info area
zen music - offline music player manager and visualizer / video/album area, two spectrum/freq analyzer for stereo channels
zen image - offline photo/video viewer and library manager / video/image area, location area, info area

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

testing - record ui events, inject ui events, check result

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




productivity centered distraction free environment
- no notifications
- no distracting ui elements
- minimalistic and beautiful design

zen desktop
- tiling window manager
- workspace oriented
- shortcut oriented
- zen key brings up workspaces and shortcuts
- zen + space brings up launcher ( dmenu )
                       brings up shortcuts ( wifi, drives, kbd)

zen button for i3 keycode press/release timeout show/hide!!! 

zen wifi
- floating wifi manager
- zen + w?

zen files
- file viewer and manager

zen music
- music player and manager

zen image
- image/video viewer and manager

zen words
- markdown based word processor

zen video
- image/video editor
- replacement for kdenlive/gimp
- replacement for adobe premier with wine
- replacement for photoshop witi wine

zen audio
- music editor
- replacement

for now
wine based photoshop
wine based premiere
wine based cubase


zen multimedia desktop environment ( ZMDE )

file manager - zen libesket mutassa, dragndrop libekbe anyagokat, egymas kozt is, flat package like file listing
zen music - itunes like,  select library, keep organized? db file at the root, keep organized - artist - album - name or just random file names with db7
songs from one year ago
activity
library logs - added two songs, moved to, etc
incomplete metadata
android sync?
preview overlay jobb also sarokban, scrollra nojon/csokkenjen, dragdroppolni lehessen ha utban van, decrease alpha

zen video - ituens like, keep organized = ha nincs metaadata akkor ev-honap-nap, vagy location, etc
zen pictures - photos like,  keep organized - ev - honap - nap, location, event, etc
zen music editor - select library, mixed - wav, sounds, etc
zen photo editor - select library, user has to place in new things
zen video editor - select library, mixed , photos, wav, videos, etc
kitty instead of alacritty

zen window manager - clipboard widget a toolbarra, tiling manager, activity bar
zen office
clojure at high level, C at low level
ffmpeg backend - ffplay audio, visualizer!
