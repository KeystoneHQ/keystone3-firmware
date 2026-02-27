#!/bin/bash
cd tools && astyle -A3nrUpHcQ --exclude=../src/cm_backtrace/Languages --exclude=../src/ui/gui_assets "../src/*.c" "../src/*.h" && cd ..