#!/bin/bash
cd tools && astyle -A3nrUpHcQ --exclude=../src/cm_backtrace/Languages "../src/*.c" "../src/*.h" && cd ..