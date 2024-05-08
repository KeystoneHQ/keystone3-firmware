# Copyright (c) 2021 W-Mai
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
# lvgl_image_converter/lv_img_conv.py
# Created by W-Mai on 2021/2/21.
# repo: https://github.com/W-Mai/lvgl_image_converter
#
##############################################################

import os
import time
from pathlib import Path
try:
    from lv_img_converter import Converter
except ImportError:
    print(
        """Please install Requirements by using `pip install -r requirements.txt`
Read README.md for more details.
          """
    )
    exit(-1)

name2const = {
    "RGB332": Converter.FLAG.CF_TRUE_COLOR_332,
    "RGB565": Converter.FLAG.CF_TRUE_COLOR_565,
    "RGB565SWAP": Converter.FLAG.CF_TRUE_COLOR_565_SWAP,
    "RGB888": Converter.FLAG.CF_TRUE_COLOR_888,
    "alpha_1": Converter.FLAG.CF_ALPHA_1_BIT,
    "alpha_2": Converter.FLAG.CF_ALPHA_2_BIT,
    "alpha_4": Converter.FLAG.CF_ALPHA_4_BIT,
    "alpha_8": Converter.FLAG.CF_ALPHA_8_BIT,
    "indexed_1": Converter.FLAG.CF_INDEXED_1_BIT,
    "indexed_2": Converter.FLAG.CF_INDEXED_2_BIT,
    "indexed_4": Converter.FLAG.CF_INDEXED_4_BIT,
    "indexed_8": Converter.FLAG.CF_INDEXED_8_BIT,
    "raw": Converter.FLAG.CF_RAW,
    "raw_alpha": Converter.FLAG.CF_RAW_ALPHA,
    "raw_chroma": Converter.FLAG.CF_RAW_CHROMA,
    "true_color": Converter.FLAG.CF_TRUE_COLOR,
    "true_color_alpha": Converter.FLAG.CF_TRUE_COLOR_ALPHA,
    "true_color_chroma": Converter.FLAG.CF_TRUE_COLOR_CHROMA,
}


def check_allowed(filepath: Path):
    suffix: str = filepath.suffix
    return suffix.lower() in [
        ".jpg",
        ".jpeg",
        ".png",
        ".bmp",
        ".tif",
        ".tga",
        ".gif",
        ".bin",
    ]


def conv_one_file(
    root: Path, filepath: Path, f, cf, ff: str, dither, bgr_mode, out_path=Path()
):
    root_path = filepath.parent
    rel_path = Path()
    if len(root_path.parts) > 0:
        rel_path = root_path.relative_to(root)
    name = filepath.stem
    conv = Converter(
        filepath.as_posix(), name, dither, name2const[f], cf_palette_bgr_en=bgr_mode
    )

    c_arr = ""
    if f in ["true_color", "true_color_alpha", "true_color_chroma"]:
        conv.convert(name2const[cf], 1 if f == "true_color_alpha" else 0)
        c_arr = conv.format_to_c_array()
    else:
        conv.convert(name2const[f])

    file_conf = {
        "C": {"suffix": ".c", "mode": "w"},
        "BIN": {"suffix": ".bin", "mode": "wb"},
    }

    out_path = root_path if out_path == Path() else out_path
    out_path = out_path.joinpath(rel_path)
    out_path.mkdir(exist_ok=True)
    out_path = out_path.joinpath(name).with_suffix(file_conf[ff]["suffix"])

    with open(out_path, file_conf[ff]["mode"]) as fi:
        res = (
            conv.get_c_code_file(name2const[f], c_arr)
            if ff == "C"
            else conv.get_bin_file(name2const[f])
        )
        fi.write(res)
    return "SUCCESS"



class ImgConverter(object):
    def __init__ (self,filepath,format,color_format,output_path):
        self.filepath = filepath
        self.f = format
        self.cf = color_format
        self.ff = "C"
        self.o = output_path
        self.r = True
        self.d = None
        self.b = True

        self.output_path = Path(self.o)

        # if output path is not exist, create it
        if not os.path.exists(self.output_path):
            os.makedirs(self.output_path)

        self.file_count = 0
        self.failed_pic_paths = []
    

    def _convert_one(self, root, file):
        print(f"{self.file_count:<5} {file} START", end="")
        t0 = time.time()
        try:
            conv_rtn = conv_one_file(
                root,
                file,
                self.f,
                self.cf,
                self.ff,
                self.d,
                self.b,
                self.output_path,
            )
            if conv_rtn == "SUCCESS":
                self.file_count += 1
                print("\b" * 5 + "FINISHED", end="")
            elif conv_rtn == "NOT ALLOWED":
                print("\b" * 5, end="")
        except Exception as e:
            print("\b" * 5, e, end="")
            self.failed_pic_paths.append(file)
        print(f" {(time.time() - t0) * 1000} ms")


    # Convert file name to CamelCase
    def convert_filename(one_string,space_character):  
        # if not contains space character, return directly
        if space_character not in one_string:
            return one_string
        string_list = str(one_string).split(space_character)   
        first = string_list[0].lower()
        others = string_list[1:]
        others_capital = [word.capitalize() for word in others]    
        others_capital[0:0] = [first]
        hump_string = ''.join(others_capital)  
        return hump_string

    def format_file_name(self):
        print("Format File Name...")
        for path in self.filepath:
            path = Path(path)
            if path.is_dir():
                path_glob = path.rglob if self.r else path.glob
                for file in path_glob("*.*"):
                    file: Path
                    if not check_allowed(file):
                        continue
                    # convert file name to camel case
                    camel_case_filename = ImgConverter.convert_filename(file.stem, "_")
                    if file.parent.name not in camel_case_filename or camel_case_filename.index(file.parent.name) != 0:
                        # camel_case_filename first char should be upper case
                        camel_case_filename = camel_case_filename[0].upper() + camel_case_filename[1:]
                        new_file_name = file.parent.name + camel_case_filename
                        new_file = file.parent.joinpath(new_file_name + file.suffix)
                        file.rename(new_file)
                    else:
                        new_file = file.parent.joinpath(camel_case_filename + file.suffix)
                        file.rename(new_file)
        print("Format File Name Complete.")

    def convert(self):
        self.format_file_name()
        for path in self.filepath:
            path = Path(path)
            if path.is_dir():
                path_glob = path.rglob if self.r else path.glob
                for file in path_glob("*.*"):
                    file: Path
                    if not check_allowed(file):
                        continue
                    self._convert_one(root=path, file=file)
            elif path.is_file():
                self._convert_one(root=path.parent, file=path)
        print()
        print(f"Convert Complete. Total convert {self.file_count} file(s).")
        print()
        if self.failed_pic_paths:
            print("Failed File List:")
            print(*self.failed_pic_paths, sep="\n")

if __name__ == "__main__":
    img_converter = ImgConverter(filepath=['./images'],format='true_color_alpha',color_format='RGB565SWAP',output_path='./test/src/ui/gui_assets')
    img_converter.convert()
