#!/bin/bash

# first copy start folder structure to test folder

echo "COPYING start TO test"

rm -r test
cp -r start test

# execute replay session on test folder

echo "STARTING TEST SESSION"

../bin/zenmusic -r ../res -c ../tst/test/cfg -p ../tst/session.rec

# compare result and test folders

echo "TEST RESULTS"

diff -r result test
