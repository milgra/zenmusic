1. FIRST START

   1.1 library popup

    manual

    1. check if config folder is non-existent ( ~/home/.config/zenmusic )
    2. start zen music
    3. library popup should appear
    4. enter a non existent folder, press enter
    5. warning should appear
    6. enter an existing folder with media files
    7. config file should be created, check it
    8. library file should be created, check it
    9. files should be parsed into library

    how to automate

    1. always start zen music with custom config file dir to not ruin the live one
    2. start zen music with api call script, (set library), check files after
    3. start zen music with gui call script with mocked api, (inject wrong path, check api call, return api result, inject good path, check api call)

2. MUSIC PLAYING

   2.1 check if music plays

       manual
   
       1. use demo library with different media types
       2. click on all of them
       3. they should play ( audible, visualizer moves)

       automatic

       1. use demo library
       2. start playing all items in song list
       3. check sound output somehow ( ffmpeg level or sndio )

   2.2 check random functionality

   2.3 check play/pause/seek
   2.4 check mute/umute/volume


   2.5 check if cover art visible


3. MUSIC FILTERING

   3.1 filtering
   
       1. click on search/query
       2. enter metallica
       3. check if metallica shows up
       4. enter lica
       5. check results
       6. check artist is metallica
       7. check artist contains tallica
       8. check bitrate is smaller than 128

   3.2 column resizing, organizing

       1. swap artist and title columns
       2. resize artist column
       3. resize last column somehow

       automate

       1. start zen music with gui call script



3. ORGANIZATION


   3.3 check if files are added after restart

       manual    

       1. add new file into music library
       2. restart zen music
       3. songs should appear in songlist, they should be playable
       4. songs should appear in library

       automated

       1. add new file into music library
       2. start zen music
       3. check library content

   3.1 check if files are moved

       manual

       1. open settings
       2. click on enable organize library
       3. accept
       4. wait for organization to finish
       5. check results
       6. check if paths in library are also overwritten

       how to automate

       1. start zen music with api call script
       2. start zen music with gui call script

   3.2 check if files are removed

       1. right click on song
       2. select delete
       3. accept popup
       4. check if file is deleted, removed from library

4. MUSIC EDITING

   4.1 tag editing

       manual

       1. use demo libary
       2. select a few songs
       3. modify one tag, remove an other
       4. accept editing
       5. check file metadata, check library metadata

       automatic

       1. use demo library
       2. api testing - inject paths, edited, removed metadata into api?
       3. gui testing - inject selection, edition, key events, accept events


5. LEAKS

   5.1 somehow log memory usage or use valgrind

AUTOMATED TESTING

- player module
- command line api calls ( library creation - compare with a good result) (playing song, writing waveform, compare) (modifying metadata - compare)?


Functional test

// songlist - songlist header

resize header cells
rearrange header cells
sort on header cell select

// songlist - query field

sort on query
