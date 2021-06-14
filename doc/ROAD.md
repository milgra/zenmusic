# Zen Music development roadmap

**before release**

 - input field into vh list in popups
 - editor popup freeze on empty list
 - cursor indicator in replay is under other layers
 - deselect should work, select range should work
 - filters window visual update
 - thinner activity window frame
 - check organize during playing
 - check select all during filtering
 - scroll over knob problem fix
 - vertical alignment of text is too on the top
 - base view resize during full screen for mouse detection, search-filter text align to middle
 - full screen button works strange
 - select range should work
 - freebsd logo should be in the center
 - dragger visual aid for column field resize
 - filters popup : genre select should filter artists, "none" item is needed
 - play/skip counter should work, last played/last skipped should work
 - set window dimensions with command line parameters
 - library popups should show actual pathes
 - scroll on press on edge of list
 - scroller event normalization to avoid lag when dragging
 - "go to current song" menu item in song context menu
 - cover art should be blank when selecting multiple songs
 - space should update play/pause button
 - filter selection scrolls item 0 under header
 - ESC should finish input field editing
 - track numbers 2 should preceed 11
 - clear button to all input fields
 - automated tests
 - fix leaks
 - youtube video

**near future**

 - ui scaling should be settable by command line parameters
 - moving mouse during inertia scroll causes scroll to stuck in some cases
 - save shuffle state, current song, current position, current volume on exit, use them on start
 - filtering with logical operators - genre is metal, year is not 2000
 - normalize css - remove unused classes, snyc element and tag and class names
 - text style should come from css
 - remove non-standard css and html (type=button, blocks=true)
 - log should fill up from up to down, should show time
 - rdft visalizer should show left/right channels in left/right visu viewwer
 - modify rdft to show more lower range
 - full screen cover art/video playing
 - full screen visualizer
 - file browser popup for library selection and cover art selection
 - analog VU meter visualizer
 - volume fade in/out on play/pause/next/prev
 - use xy_new and xy_del everywhere for objects that have to be released
 - solve last column resize problem
 - select/copy/paste in textfields
 - prev button in shuffle mode should jump to previously played song

**inbetween future**

 - statistics - top 10 most listened artist, song, genre, last month, last year, etc - stats browser
 - songs from one year ago this day - history browser
 - dark mode
 - metadata update should happen in the backgroun to stop ui lag
 - vertical limit for paragraphs to avoid texture map overrun
 - settings cell autosize?

**far future**

 - library analyzer should avoid extension-based analyzation, use something deeper
 - cerebral cortex as interactive visualizer - on left/right press start game
 - andromeda : monolith 64K demo like particle visualizer	     
 - grid-based warping of video/album cover based on frequency ( bass in the center )
 - speed up font rendering by using glyph indexes instead of codepoints
 - vh_textinput should seamlessly switch between texture paragraph and glpyh-based paragraph
 - vh_textinput autoscroller, vertical and horizontal
 - Android Auto support with full screen visualizer for kick-ass experience
 - Vulkan backend for Zen UI