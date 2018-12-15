#!/usr/bin/env bash
set -e
ctr=0
while [[ 1 ]]; do
    ((ctr++))
    echo "********TEST $ctr ******";
    time(echo $1 | ../cmake-build-debug/src/tests/nested_mutex_test);
done
