# -*- coding: utf-8 -*-
# !/usr/bin/python

import argparse
import csv
import re
import os
import shutil
import subprocess

from pathlib import Path

g_font_size = 0

def update_font_properties(file_path, font_size):
    font_properties = {
        20: (30, 7),
        24: (40, 11),
        28: (40, 9),
        36: (37, 0)
    }

    line_height, base_line = font_properties.get(font_size, (None, None))
    if line_height is None or base_line is None:
        print(f"No properties found for font_size {font_size}.")
        return

    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    import re
    content = re.sub(r'\.line_height = \d+,\s*/\*\s*The maximum line height required by the font\s*\*/',
                     f'.line_height = {line_height},          /*The maximum line height required by the font*/', content)
    content = re.sub(r'\.base_line = \d+,\s*/\*\s*Baseline measured from the bottom of the line\s*\*/',
                     f'.base_line = {base_line},             /*Baseline measured from the bottom of the line*/', content)

    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(content)
    print(f"Updated {file_path} for font_size {font_size} with line_height {line_height} and base_line {base_line}.")

def find_lv_font_conv():
    configured = os.environ.get("LV_FONT_CONV")
    if configured:
        return configured

    executable = shutil.which("lv_font_conv")
    if executable:
        return executable

    nvm_root = Path.home() / ".nvm" / "versions" / "node"
    candidates = sorted(nvm_root.glob("*/bin/lv_font_conv"), reverse=True)
    if candidates:
        return str(candidates[0])

    raise FileNotFoundError(
        "lv_font_conv was not found; install it or set LV_FONT_CONV"
    )


def build_lv_font_conv_command(bpp, size, font, symbols, output_file):
    return [
        find_lv_font_conv(),
        "--bpp", str(bpp),
        "--size", str(size),
        "--no-compress",
        "--font", font,
        "--symbols", symbols,
        "--format", "lvgl",
        "-o", output_file,
    ]

def parse_command_line(command_line="cmd_tool --bpp 8 --size 12 --font Arial.ttf --symbols ABCD --format xyz", font_size=None, language=None, unique_characters=None, label=None):
    symbols = re.search(r"--symbols (.+?) --format", command_line).group(1)
    options = {
        'bpp': re.search(r"--bpp (\d+)", command_line).group(1),
        'size': int(re.search(r"--size (\d+)", command_line).group(1)),
        'font': re.search(r"--font ([\w-]+\.ttf)", command_line).group(1),
        # Older generated files contain an unmatched quote plus padding spaces
        # because the command used to be assembled through zsh.
        'symbols': symbols.strip().strip('"')
    }
    
    if font_size in [20, 24]:
        bpp = 2
    elif font_size in [28, 36]:
        bpp = 1

    output_file = "../gui_assets/font/" + language + "/" + label
    try:
        with open(output_file, 'r', encoding='utf-8') as generated_font:
            has_space_glyph = '/* U+0020 " " */' in generated_font.read()
    except FileNotFoundError:
        has_space_glyph = False

    if options['symbols'] != unique_characters or not has_space_glyph:
        font_mapping = {
            'cn': 'NotoSansSC-Regular.ttf',
            'ko': 'NotoSansKR-Regular.ttf',
            'ru': 'NotoSans-Regular.ttf',
            'es': 'NotoSans-Regular.ttf',
            'de': 'NotoSans-Regular.ttf',
            'ja': 'NotoSansJP-Regular.ttf',
        }
        # Space is intentionally removed from unique_characters below so it does
        # not affect the symbols comparison. It still needs to be included in
        # every generated font: LVGL does not automatically fall back to another
        # font for U+0020, and a missing space is rendered as the missing-glyph
        # box between translated words.
        symbols_for_generation = unique_characters + " "
        build_command = build_lv_font_conv_command(
            bpp,
            font_size,
            font_mapping[language],
            symbols_for_generation,
            output_file,
        )
        cmd_result = subprocess.run(build_command, check=False)
        if cmd_result.returncode != 0:
            raise RuntimeError(
                f"lv_font_conv failed with exit code {cmd_result.returncode}"
            )
        update_font_properties(output_file, font_size)
        # raise ValueError("Unique characters do not match the symbols provided in the command line.")

    return options, language

def extract_unique_characters(df, font_size, column):
    additional_chars = {
        24: "·QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@",
        20: "·QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@",
        28: "·QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@",
        36: "·QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@",
    }
    unique_chars = set(additional_chars.get(font_size, additional_chars[28]))
    for row in df:
        try:
            row_font_size = int(row['font'])
        except (KeyError, TypeError, ValueError):
            continue
        value = row.get(column)
        if row_font_size == font_size and value:
            unique_chars.update(set(value))
    text = ''.join(sorted(unique_chars))
    text = text.replace('\"', '')
    text = text.replace('\n', '')
    text = text.replace(' ', '')
    return text

def main():
    for language in ['cn', 'ko', 'ru', 'es', 'de', 'ja']:
        try:
            with open('data.csv', newline='', encoding='utf-8') as csv_file:
                df = list(csv.DictReader(csv_file))
            font_labels = {
                20: f"{language}Illustrate",
                24: f"{language}Text",
                28: f"{language}LittleTitle",
                36: f"{language}Title"
            }
            for font_size in [20, 24, 28, 36]:
                g_font_size = font_size
                unique_characters = extract_unique_characters(df, font_size, language)
                label = font_labels.get(font_size, f"Unknown Font Size {font_size}")
                source_file_path = Path("../gui_assets/font") / language / f"{label}.c"
                try:
                    with open(source_file_path, 'r', encoding='utf-8') as file:
                        lines = file.readlines()
                        if len(lines) >= 4:
                            parse_command_line(lines[3].strip(), font_size, language, unique_characters, f"{label}.c")
                        else:
                            print(f"The file {source_file_path} does not have a fourth line.")
                except FileNotFoundError:
                    print(language)
                    try:
                        with open(source_file_path, 'w', encoding='utf-8') as file:
                            parse_command_line(font_size = font_size, language = language, unique_characters = unique_characters, label = f"{label}.c")
                    except FileNotFoundError:
                        print(f"The file {source_file_path} does not exist.")
        except Exception as e:
            print("language is: g_font_size = ", language, g_font_size)
            print("An error occurred:", e)

if __name__ == '__main__':
    main()
