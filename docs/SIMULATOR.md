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
> rustup install nightly-2023-12-01

`cbindgen` is also required to build essential C header files for Rust libs, so run the following after rust setup: 
> cargo install cbindgen

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

## Some tricks

### Transaction Layout File
In `src/ui/gui_analyze/gui_analyze.c` you will see code like this:
```C
{
        REMAPVIEW_BTC,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"btc_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxDetail\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_btc.json",
#endif
        GuiGetParsedQrData,
        NULL,
        FreePsbtUxtoMemory,
    },
```

This is a transaction layout JSON, the code says if you are not compiling simulator, it will read the hardcoded JSON text. Or it will read the file at `PC_SIMULATOR_PATH "/page_btc.json"`, the `PC_SIMULATOR_PATH` actually points to the folder `ui_simulator/assets`, so this file is actually at `ui_simulator/assets/page_btc.json`.

You will find that the file is not exists, so you need to parse this JSON text by your own then create a `page_btc.json` in that folder. You can use the script `ui_simulator/tools/read_then_write_json.mjs` to do the stuff but you need to change the code inside because the parameters are hardcoded now. 
> cd ui_simulator & node tools/read_then_write_json.mjs

### Reading QR code
Simulating the camera is difficult, but we have a workaround.

A file `ui_simulator/assets/qrcode_data.txt` is created when you run the simulator build.

You can scan the QR codes with your phone or other scanner tools then paste the text into the `qrcode_data.txt`, then click the scanner button in the home page, it will reads the data in this file then continue process the data.

If your QR code is animating, just read them by your phone and paste the data line by line. Most QR codes start with `UR` and have a format of `UR:{NAME}/M-N`, UR(Please read [this](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md)) is a fountain code generator so `M` indicates the index of this qr code, and `N` indicates the how many qr codes construct the original data.

In theory you will need to collect `1.75 * N` number of qr codes to recover the original data, in our practice it should be `2N`. for example, if you scanned your animating QR code and see it shows `UR:Bytes/1-3/XXXXXXXXXXXXXXXXXXXXXX`, it says you'd better collect 5 more qr codes and paste them into the `qrcode_data.txt` like:

```
UR:Bytes/1-3/XXXXXXXXXXXXXXXXXXXXXX
UR:Bytes/5-3/XXXXXXXXXXXXXXXXXXXXXX
UR:Bytes/7-3/XXXXXXXXXXXXXXXXXXXXXX
UR:Bytes/11-3/XXXXXXXXXXXXXXXXXXXXXX
UR:Bytes/12-3/XXXXXXXXXXXXXXXXXXXXXX
UR:Bytes/17-3/XXXXXXXXXXXXXXXXXXXXXX
```

Also do not leave blank lines in the file.