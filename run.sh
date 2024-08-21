#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd $SCRIPT_DIR

rm -fr ./ga-output ; mkdir ./ga-output
./build/analyzer --bcfiles-dir ./testprog/build --output-dir ./ga-output
