#!/bin/bash

# first copy start folder structure to result folder

echo "COPYING start TO test"

rm -r tst/test
rm -r tst/result
cp -r tst/start tst/test

# execute record session on result folder

echo "STARTING RECORDING SESSION"

cnt=0
res="y"

while [ $res = "y" ]; do

    res_path="../res"
    cfg_path="../tst/test/cfg"
    ses_path="../tst/session$cnt.rec"
    
    ((cnt+=1))
    
    bin/zenmusic -r $res_path -c $cfg_path -s $ses_path

    echo "Record another session? y/n"

    read res

done

echo "RENAMING test to result"

mv tst/test tst/result

echo "RECORDING FINISHED"
