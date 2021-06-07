# Zen Music music player and organizer test protocol

---

How to record a session :
bin/zenmusicdev -c /home/milgra/Downloads/ZMTestConfig -s ../tst testrecord.rec  

How to play a session :
bin/zenmusicdev -c /home/milgra/Downloads/ZMTestConfig -p ../tst   

How to check for memory leaks :
valgrind --leak-check=full --show-leak-kinds=all --suppressions=valgrind.supp bin/zenmusicdev

---

start with no config file

 - default config should be created
 - explicit config file should be created if added as option
 - library popup should show up
 - library popup should show error in case of invalid library
 - library popup should disappear in case of valid library
 - library should be read, songs should be analyzed
 - log should be visible in activity view

start with config file and library

 - library should be read, songs should be analyzed, lib state and db state should be synced
 - (add, remove, rename songs and check if they are noticed after restart )

songlist

 - song list should be scrollable
 - song list should show valid values
 - columns should be resizable/rearrangable
 - scrollable rectangle should be under table header, over bottom scroller and visualizer, left to right scroller

play controls

 - volume control knob should work with scroll, click, move
 - mute button should work
 - shuffle button should work
 - prev/next button should work
 - play/pause button should work
 - seek control knob should work with scroll, click, move
 - time display should work
 - song playing should work
 - video playing should work
 - cover art viewer should work
 - visualizer should work
 - play count/skin count/last played/last skipped should work

filter

 - filters button should work
 - filter bar should work
 - clean filter bar button should work
 - entering text in filter bar should work

settings

 - settings popup should show up
 - settings popup should show correct values
 - toggling organize lib should organize lib
 - toggling remote control should toggle remote control
 - changing library path should change library

about popup

 - clicking on links should open them in the browser

metadata editing

 - check if cover art is visible
 - add cover to mp3, mp4, m4a withot cover, check resulting song
 - add cover to mpo3, mp4, m4a with cover, check resulting song

 - metadata field modification, check resulting song, check db
 - metadata field creation, check resulting song, check db
 - metadata field deletion, check resulting song, check db

 - metadata update leak, check leaks after heavy metadata updates

ui renderer

 - test texture resize and reset, texture overflow and reset
