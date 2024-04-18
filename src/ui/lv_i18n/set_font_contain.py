# -*- coding: utf-8 -*-
# !/usr/bin/python

import pandas as pd

df = pd.read_csv('data.csv')

def extract_unique_characters(df, font_size, column):
    if font_size == 24:
        additional_chars = "QWERTYUIOPASDFGHJKLZXCVBNM,/:\";'[]<>~!@#$%^&*()_+=0987654321·qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]{}#%^*+=_\\|~<>€£¥·-/:;()$&`.?!'@"
        unique_chars = set(additional_chars)
    else:
        additional_chars = "0987654321"
        unique_chars = set(additional_chars)
    subset = df[df['font'] == font_size][column].dropna()
    subset.apply(lambda x: unique_chars.update(set(x)))
    return ''.join(sorted(unique_chars))

# languages = ['Russian', 'Korean', 'cn']
languages = ['cn']

for language in languages:
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