#!/bin/bash

version="$1"
build=0

# break down the version number into it's components
regex="([0-9]+)"
if [[ $version =~ $regex ]]; then
  build="${BASH_REMATCH[1]}"
fi

# check paramater to see which number to increment
build=$(echo $build + 1 | bc)

# echo the new version number
echo "${build}"
