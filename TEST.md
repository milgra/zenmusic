# Zen Music test protocol

Before creating a pull request first check for leaks around your modification with valgrind(freebsd) or valgrind/address sanitizer(linux)
Then create a release build with gmake/make rel.
Then run all test sessions with gmake/make runtest.
Check the diff log after test session finished. Only the dates in zemusic.kvl should differ, media files, library structure and screenshots shouldn't differ.

---

How to record a test session :

gmake/make rectest

or

bin/zenmusic -r ../res -c ../tst/test/cfg -s ../tst/session.rec  

How to run a test session :

gmake/make runtest

or

bin/zenmusic -r ../res -c ../tst/test/cfg -p ../tst/session.rec  

How to check for memory leaks :

valgrind ( freebsd/linux )

valgrind --leak-check=full --show-leak-kinds=all --suppressions=tst/valgrind.supp bin/zenmusicdev

address sanitizer ( linux )

TODO

## Protocol

If you are re-recording the main test session, follow this protocol.
If you add a new feature please add it to a proper place in the protocol.
If you see SCREENSHOT take a screenshot by pressing PRTINSCREEN button

SESSION 0

1. start with no config file

 - start app with valid res_path and cfg_path, cfg_path shouldn't contain any config file
 - library popup should show up SCREENSHOT
 - press reject button, app should close
 - answer "y" in terminal to "Record another session?"

SESSION 1

2. start with no config file ( cfg_path shouldn't contain any config file )

 - start app with valid res_path and cfg_path, cfg_path shouldn't contain any config file
 - library popup should who up
 - enter "invalid", library popup should show error in case of invalid library SCREENSHOT
 - enter "../tst/test/lib1", library popup should disappear in case of valid library, library should be read, songs should be analyzed
 - last log message should be visible in main display - SCREENSHOT
 - click on main display, activity log popup should appear, should show logs - SCREENSHOT
 - click outside activity log popup

3. library browser

 - song list should show valid values SCREENSHOT
 - song list should be scrollable
 - scroll to top
 - columns should be resizable/rearrangable SCREENSHOT
 - scroll song list next to bottom visualizer rectangles, over right and bottom scroller to see if they don't block events

4. filtering

 - filters button should pop up filters popup SCREENSHOT
 - gerne/altist can be selected, it should show up in filter bar SCREENSHOT
 - filter bar should be selectable, editable, it should filter library browser
 - clean filter bar button should work

5. settings

 - click on settings icon ( vertical bars ), settings popup should show up, it should show correct values SCREENSHOT
 - click on library path, cancel popup
 - click on library path, switch to lib2 at ../tst/test/lib2 ( library gets read in the background, will be checked after session )
 - click on library path, switch to lib1 at ../tst/test/lib1  ( library gets read in the background, will be checked after session )
 - library browser' content should change SCREENSHOT
 - click on organize library, cancel popup
 - click on organize library, accept popup ( library gets re-organized in the background, will be checked after session )
 - click on remote control, cancel popup
 - click on remote control, accept popup ( TODO - test somehow!!! )
 - click on config path, popup should show up SCREENSHOT
 - cancel popup, click outside settings popup
 - click on close app icon
 - answer "y" in terminal to "Record another session?"

SESSION 2

6. start with existing cfg file and existing organized library

 - start app with valid res_path and cfg_path and existing, organized library

7. about popup

 - click on about icon ( heart ), about popup should show up SCREENSHOT
 - click on support on patrean, popup should pop up SCREENSHOT
 - accept popup, click outside about popup

8. metadata editor

 - click on tag editor button ( horizontal bars ), popup should show up, it should show correct values SCREENSHOT
 - check path, it should be relative path after organized enabled SCREENSHOT
 - click on cancel button, popup should disappear
 - click on tag editor button again
 - click on date field, add 1974
 - click on comments field, add "best song"
 - click on accept button ( media file is updated in the background )

 - in library browser, select second song, click on tag editor
 - click on "add new image"
 - click on cancel
 - click on "add new image"
 - enter ../tst/vader.jpeg
 - click on accept, image shoudl show up in cover view SCREENSHOT
 - click on accept, text should fit in warning popup SCREENSHOT
 - click on reject warning popup
 - click on accept, accept warning popup
 - tag editor popup should disappear

 - in library browser, select ADELE nocover metadata
 - in Artist field, type in "Adele"
 - accept edit, library browser row should be updated, SCREENSHOT ( song gets reorganized in the background, will be checked after session )

9. library context popup

 - right click on fourth song
 - click select all SCREENSHOT
 - right click on selection
 - click edit song info
 - editor popup with multiple items should appear, album art should be blank, SCREENSHOT
 - click cancel
 - click on last song
 - click on delete
 - click on reject
 - click on last song
 - click on delete
 - click on accept ( shong gets deleted from library ,will be checked after session )
 - song should disappear from list, log should be visible in main display SCREENSHOT
>
10. ui renderer

 - switch to full screen and back with keys ( CMD+F ) SCREENSHOT when in normal size ( texture map gets reset in background )
 - switch to full screen and back with full screen button SCREENSHOT when in normal size ( texture map gets reset in background )

11. play controls

 - double click on ADELE_SIL in library, move mouse continuously to avoid inactiveness skipping in recorder, after 2 seconds, pause with SPACE, time displays should work, main display should show song info SCREENSHOT
 - press play and press play again after 2 seconds SCREENSHOT

 *THE FOLLOWING CANT BE CONFIRMED AUTOMATICALLY*

 - double click on ROYKSOPP, video should be visible in cover art view
 - double click on Odyssey, song should be hearable, visualizer active
 - move mouse over visualizer rects, switch visualization
 - seek forward and backward over play button with scroll
 - seek forward and backward by pressing on seek ring around play btn
 - seek forward and backward by dragging on seek ring aroung play button
 - press mute button, press again - audio should disappear and re-appear
 - set volume by scrolling over volume knob
 - set volume by pressing on volume ring
 - set volume by dragging on volume ring
 - press prev/next button, songs should start playing
 - press shuffle button, press prev/next button, songs should be played randomly
 - play count/skin count/last played/last skkipped should be updated in library browser

Post-test checks

 - config file should be present under config directory, should contain correct values
 - library structure should mirror what you did under the session
 - database file should contain correct values

What should be ensured after recording a new session

 - lib 1 should be organized, artist - album folders should be created
 - metadata should reflex what was done in the session
 - play/skip counters should work, last player/last skipped should work