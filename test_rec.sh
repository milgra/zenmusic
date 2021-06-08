#!/bin/bash

# first copy start folder structure to result folder
cp -r tst/start tst/result

# execute record session
bin/zenmusic -r ../res -c ../tst/result/cfg -s ../tst/session.rec
