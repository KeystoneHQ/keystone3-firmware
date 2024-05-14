# -*- coding: utf-8 -*-
# !/usr/bin/python

import csv
import yaml
import argparse
import subprocess
import glob
import os
import re
import get_font_contain

compile_command = 'lv_i18n compile -t "*.yml" -o .'

def replace_text_in_file(file_path = 'lv_i18n.c'):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    modified_content = re.sub(r'\bstatic lv_i18n_phrase_t\b', 'const static lv_i18n_phrase_t', content)

    if content != modified_content:
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(modified_content)
        print("File has been modified.")

with open("./data.csv", newline="", encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    en = {}
    ru = {}
    cn = {}
    ko = {}
    es = {}
    ge = {}
    ja = {}

    for row in reader:
        id = row['ID']
        en[id] = row['en']
        ru[id] = row['ru']
        cn[id] = row['cn']
        ko[id] = row['ko']
        es[id] = row['es']
        ge[id] = row['ge']
        ja[id] = row['ja']

with open("./en.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'en': en}, f, default_flow_style=False)

with open("./ru.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'ru': ru}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l ru'

with open("./zh-CN.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'zh-CN': cn}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l zh-CN'

with open("./ko.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'ko': ko}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l ko'

with open("./es.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'es': es}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l es'

with open("./de.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'de': de}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l de'

with open("./ja.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'ja': ja}, f, allow_unicode=True, default_flow_style=False)
compile_command += ' -l ja'

compile_command += ' -l en'

print(compile_command)

cmd_result = os.system(compile_command)
if cmd_result != 0:
    exit(cmd_result)

replace_text_in_file()
get_font_contain.main()