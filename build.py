# -*- coding: utf-8 -*-
# !/usr/bin/python

import shutil
import platform
import subprocess
import argparse
import os

source_path = os.path.dirname(os.path.abspath(__file__))
build_dir = "build"
build_path = source_path + '/' + build_dir

argParser = argparse.ArgumentParser()
argParser.add_argument("-e", "--environment", help="please specific which environment you are building, dev or production")
argParser.add_argument("-p", "--purpose", help="please specific what purpose you are building, set it to `debug` for building unsigned firmware.")
argParser.add_argument("-o", "--options", nargs="?", help="specify the required features you are building")
argParser.add_argument("-t", "--type", help="please specific which type you are building, btc_only or general")
argParser.add_argument("-f", "--force", action="store_true" , help="force to build the image even if there is no change in the images")
argParser.add_argument("-s", "--skip-image", action="store_true", help="skip building images")
def build_firmware(environment, options, bin_type):
    is_release = environment == "production"
    is_btc_only = bin_type == "btc_only"
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    padding_script = os.path.join(build_dir, "padding_bin_file.py")
    if not os.path.exists(padding_script):
        shutil.copy(os.path.join("tools/padding_bin_file", "padding_bin_file.py"), build_dir)

    os.chdir(build_path)

    if platform.system() == 'Darwin':
        cmd = 'cmake -G "Unix Makefiles" .. -DLIB_RUST_C=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++'
    else:
        cmd = 'cmake -G "Unix Makefiles" .. -DLIB_RUST_C=ON'
    if is_release:
        cmd += ' -DBUILD_PRODUCTION=true'
        cmd += ' -DRU_SUPPORT=true'
    if is_btc_only:
        cmd += ' -DBTC_ONLY=true'


    for option in options:
        if option == "screenshot":
            cmd += ' -DENABLE_SCREEN_SHOT=true'
        if option == "debugmemory":
            cmd += ' -DDEBUG_MEMORY=true'
        if option == "simulator":
            cmd += ' -DCMAKE_BUILD_TYPE=Simulator'
        # add more option here.

    cmd += " -DRU_SUPPORT=true"

    cmd_result = os.system(cmd)
    if cmd_result != 0:
        return cmd_result
    make_result = os.system('make -j')
    if make_result != 0:
        return make_result
    return os.system('python padding_bin_file.py mh1903.bin')


def ota_maker():
    os.chdir(source_path)
    if platform.system() == 'Darwin':
        cpu = platform.processor()
        if cpu == "arm":
            args = ("./tools/mac_arm/ota-maker", "--source", "./build/mh1903.bin", "--destination", "./build/keystone3.bin")
        else:
            args = ("./tools/mac/ota-maker", "--source", "./build/mh1903.bin", "--destination", "./build/keystone3.bin")
    popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    popen.wait()

def build_img_to_c_file(force=False, skip_image=False):
    if skip_image:
        print("Skip building the images.")
        return
    import hashlib
    def calculate_directory_hash(directory):
        hash_md5 = hashlib.md5()
        for root, dirs, files in os.walk(directory):
            for file in files:
                if file.lower().endswith('.png'):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'rb') as f:
                        for chunk in iter(lambda: f.read(4096), b''):
                            hash_md5.update(chunk)
        return hash_md5.hexdigest()

    def load_previous_hash(hash_file):
        if os.path.exists(hash_file):
            with open(hash_file, 'r') as f:
                return f.read().strip()
        return None
    
    def save_current_hash(hash_file, current_hash):
        with open(hash_file, 'w') as f:
            f.write(current_hash)

    def build_images(images_directory):
        from img_converter import ImgConverter
        img_converter = ImgConverter(filepath=[images_directory],format='true_color_alpha',color_format='RGB565SWAP',output_path='./src/ui/gui_assets')
        img_converter.convert()

    hash_file  = "./src/ui/gui_assets/images_hash.txt"
    images_directory = "./images"

    previous_hash = load_previous_hash(hash_file)
    current_hash = calculate_directory_hash(images_directory)

    if force or previous_hash != current_hash:
        if force:
            print("Force to build the image.")
        save_current_hash(hash_file, current_hash)
        build_images(images_directory)
    else:
        print("No image changes detected, skip building images.")

if __name__ == '__main__':
    args = argParser.parse_args()
    print("=============================================")
    print("--")
    print(f"Building firmware for { args.environment if args.environment else 'dev'}")
    print(f"Building firmware type { args.type if args.type else 'general'}")
    if args.options: 
        print(f"Options: {args.options}")
    print("--")
    print("=============================================")
    env = args.environment
    options = []
    if args.options:
        options = args.options.split(",")
    bin_type = args.type
    shutil.rmtree(build_path, ignore_errors=True)
    build_img_to_c_file(args.force, args.skip_image)
    build_result = build_firmware(env, options, bin_type)
    if build_result != 0:
        exit(1)
    if platform.system() == 'Darwin':
        ota_maker()
    purpose = args.purpose
    if purpose and purpose == "debug":
        ota_maker()

