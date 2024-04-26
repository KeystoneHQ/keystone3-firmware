# -*- coding: utf-8 -*-
# !/usr/bin/python

import csv
import yaml
import argparse
import subprocess
import glob
import os

parser = argparse.ArgumentParser(description='Convert CSV to YAML')
parser.add_argument('--ru', action='store_true', help='Generate Russian translations')
parser.add_argument('--zh', action='store_true', help='Generate Chinese (Simplified) translations')
args = parser.parse_args()
compile_command = 'lv_i18n compile -t "*.yml" -o .'

with open("./data.csv", newline="", encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    en = {}
    ru = {}
    cn = {}

    for row in reader:
        id = row['ID(0.0.1)']
        en[id] = row['en']
        if args.ru:
            ru[id] = row['Russian']
        if args.zh:
            cn[id] = row['cn']

with open("./en.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'en': en}, f, default_flow_style=False)

if args.ru:
    with open("./ru.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'ru': ru}, f, allow_unicode=True, default_flow_style=False)
    compile_command += ' -l ru'

# if args.zh:
#     with open("./zh-CN.yml", 'w', encoding='utf-8') as f:
#         yaml.dump({'zh-CN': cn}, f, allow_unicode=True, default_flow_style=False)
#     compile_command += ' -l zh-CN'

compile_command += ' -l en'

print(compile_command)

cmd_result = os.system(compile_command)
if cmd_result != 0:
    exit(cmd_result)