#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd $SCRIPT_DIR

./build/analyzer --bcfiles-dir ./testprog/ --output-dir ./ga-output
