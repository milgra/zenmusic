#!/bin/bash

# first copy start folder structure to test folder
cp -r tst/start tst/test

# execute test session
bin/zenmusic -r ../res -c ../tst/test/cfg -p ../tst/test1.session

# compare final and test folders
diff -r tst/final tst/test   
