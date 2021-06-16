# Zen Music development roadmap

**before release**

 - play/skip counter should work, last played/last skipped should work
 - multiple texture full on drag scroll
 - manual test
 - automated tests
 - fix leaks
 - youtube video

**near future**

 - use zen_callback in all view handlers
 - set window dimensions with command line parameters
 - filters popup : genre select should filter artists, "none" item is needed
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
 - Android Auto support with full screen visualizer for kick-ass experience
 - Vulkan backend for Zen UI