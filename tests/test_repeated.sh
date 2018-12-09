#!/usr/bin/env bash
set -e
while [[ 1 ]]; do
    echo "********TEST******";
    echo $1 | ../cmake-build-debug/src/tests/nested_mutex_test;
done
