#!/bin/bash

# first copy start folder structure to test folder

echo "COPYING tst/start TO tst/test"

rm -r tst/test
cp -r tst/start tst/test

# execute replay session on test folder

echo "STARTING TEST SESSION"

bin/zenmusic -r ../res -c ../tst/test/cfg -p ../tst/session.rec

# compare result and test folders

echo "TEST RESULTS"

diff -r tst/result tst/test
