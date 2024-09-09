# -*- coding: utf-8 -*-
# !/usr/bin/python

import argparse
import pandas as pd
import re
import os

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

def build_lv_font_conv_command(bpp, size, font, symbols, output_file):
    command = "lv_font_conv"
    command += f" --bpp {bpp}"
    command += f" --size {size}"
    command += " --no-compress"
    command += f" --font {font}"
    command += f" --symbols {symbols}"
    command += " --format lvgl"
    command += f" -o {output_file}"

    return command

def parse_command_line(command_line="cmd_tool --bpp 8 --size 12 --font Arial.ttf --symbols ABCD --format xyz", font_size=None, language=None, unique_characters=None, label=None):
    options = {
        'bpp': re.search(r"--bpp (\d+)", command_line).group(1),
        'size': int(re.search(r"--size (\d+)", command_line).group(1)),
        'font': re.search(r"--font ([\w-]+\.ttf)", command_line).group(1),
        'symbols': re.search(r"--symbols (.+?) --format", command_line).group(1)
    }
    
    if font_size in [20, 24]:
        bpp = 2
    elif font_size in [28, 36]:
        bpp = 1

    if options['symbols'] != unique_characters:
        font_mapping = {
            'cn': 'NotoSansSC-Regular.ttf',
            'ko': 'NotoSansKR-Regular.ttf',
            'ru': 'NotoSans-Regular.ttf',
            'es': 'NotoSans-Regular.ttf',
            'de': 'NotoSans-Regular.ttf',
            'ja': 'NotoSansJP-Regular.ttf',
        }
        if os.environ.get('SHELL') == '/bin/zsh':
            unique_characters = '\"' + '\\\"' + unique_characters + " " + "\""
            unique_characters = unique_characters.replace("`","\\`")
        else:
            unique_characters = '\"\"\"' + unique_characters + " " +'\"'
        build_command = build_lv_font_conv_command(bpp, font_size, font_mapping[language], unique_characters, "../gui_assets/font/" + language + "/" + label)
        cmd_result = os.system(build_command)
        if cmd_result != 0:
            exit(cmd_result)
        update_font_properties("../gui_assets/font/" + language + "/" + label, font_size)
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
    subset = df[df['font'] == font_size][column].dropna()
    subset.apply(lambda x: unique_chars.update(set(x)))
    text = ''.join(sorted(unique_chars))
    text = text.replace('\"', '')
    text = text.replace('\n', '')
    text = text.replace(' ', '')
    return text

def main():
    for language in ['cn', 'ko', 'ru', 'es', 'de', 'ja']:
        try:
            df = pd.read_csv('data.csv')
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