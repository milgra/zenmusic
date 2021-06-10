#!/bin/bash

# first copy start folder structure to result folder

echo  "COPYING start TO test"

rm -r tst/test
cp -r tst/start tst/test

# execute record session on result folder

echo "STARTING RECORDING SESSION"

bin/zenmusic -r ../res -c ../tst/test/cfg -s ../tst/session.rec

echo "RENAMING test to result"

mv test result
