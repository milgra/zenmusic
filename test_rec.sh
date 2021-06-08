#!/bin/bash

# first copy start folder structure to result folder

echo  "COPYING tst/start TO tst/test"

rm -r tst/test
cp -r tst/start tst/test

# execute record session on result folder

echo "STARTING RECORDING SESSION"

bin/zenmusic -r ../res -c ../tst/test/cfg -s ../tst/session.rec

echo "RENAMING tst/test to tst/result"

mv tst/test tst/result
