# -*- coding: utf-8 -*-
# !/usr/bin/python

import argparse
import pandas as pd

ADDITIONAL_CHARS_LARGE = "·QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@"
ADDITIONAL_CHARS_SMALL = "0987654321/"

def extract_unique_characters(df, font_size, column):
    unique_chars = set(ADDITIONAL_CHARS_LARGE if font_size in [24, 20] else ADDITIONAL_CHARS_SMALL)
    subset = df[df['font'] == font_size][column].dropna()
    subset.apply(lambda x: unique_chars.update(set(x)))
    return ''.join(sorted(unique_chars))

def main():
    parser = argparse.ArgumentParser(description='Convert CSV to YAML')
    parser.add_argument('--ru', action='store_true', help='Generate Russian translations')
    parser.add_argument('--cn', action='store_true', help='Generate Chinese (Simplified) translations')
    parser.add_argument('--ko', action='store_true', help='Generate Korean translations')
    args = parser.parse_args()

    language = 'cn'
    if args.ru:
        language = 'ru'
    elif args.cn:
        language = 'cn'
    elif args.ko:
        language = 'ko'

    df = pd.read_csv('data.csv')

    unique_characters_font_20 = extract_unique_characters(df, 20, language)
    unique_characters_font_24 = extract_unique_characters(df, 24, language)
    unique_characters_font_28 = extract_unique_characters(df, 28, language)
    unique_characters_font_36 = extract_unique_characters(df, 36, language)

    print(f"\nUnique characters in {language} with font size {language}Illustrate:")
    print(unique_characters_font_20)
    print(f"Unique characters in {language} with font size {language}Text:")
    print(unique_characters_font_24)
    print(f"\nUnique characters in {language} with font size {language}LittleTitle:")
    print(unique_characters_font_28)
    print(f"Unique characters in {language} with font size {language}Title:")
    print(unique_characters_font_36)
    print("\n" + "-"*50 + "\n")

if __name__ == '__main__':
    main()