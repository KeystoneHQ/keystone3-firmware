# Simulator on MacOS

## Setup

### 0. Open this project with VSCode

and agree all things

### 1. Install SDL Driver

`brew install sdl2`

## How to Use

### 1. Build

`pio debug --environment emulator_64bits`

it will create a runnable bin `.pio/build/emulator_64bits/program`

### 2. Run

You can just run the binary by command

`.pio/build/emulator_64bits/program`

It will print basic message on the terminal.

#### OR

You can use `lldb` to have a better debug experience.

```
# execute lldb
lldb

# source setup script
(lldb) command source .lldbinit

# exit
# first close the program window and type
exit
```

You can automate the process above by

`echo "settings set target.load-cwd-lldbinit true" > ~/.lldbinit`
