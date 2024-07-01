# Simulator
This article describes how to use simulator to ease your development.

## Requirements
Python3, Rust, SDL2, C compiler.

### Python3
Please follow Python3 official site: https://www.python.org/downloads/

We recommend creating a virtual env for this project: https://docs.python.org/3/library/venv.html

### Rust
Please follow Rust official site: https://www.rust-lang.org/tools/install

We have fixed the rust version in most of our rust libs so please run this command when you finish rust setup: 
> rustup install nightly-2023-06-26

### SDL2
We use SDL2(https://www.libsdl.org/) to create our simulator runnable, so you need to setup SDL2 to run the simulator compilation.

for Linux:
> apt install libsdl2-dev libsdl2-2.0-0

for MacOS:
> brew install sdl2

### C compilers
Your local C compilers might be enough to build simulator. If you don't have a C compiler, please install it via homebrew, apt or other package managers. Or install GCC by yourself: https://gcc.gnu.org/install/

## Usage
0. Setup your Python environment.
    > pip3 install -r requirements.txt

1. run `python3 build.py -o simulator` at the project root.

2. run `build/simulator` or `lldb` if you want to debug with lldb.
