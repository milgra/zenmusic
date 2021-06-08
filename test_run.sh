#!/bin/bash

# first copy start folder structure to test folder
cp -r tst/start tst/test

# execute replay session
bin/zenmusic -r ../res -c ../tst/test/cfg -p ../tst/session.rec

# compare result and test folders
diff -r tst/result tst/test   
