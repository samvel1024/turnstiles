#!/usr/bin/env bash
set -e
ctr=0
while true; do
    ctr=$((ctr + 1))
    echo "********TEST $ctr ******";
    echo "$1" | ../cmake-build-debug/src/tests/"$2";
done
