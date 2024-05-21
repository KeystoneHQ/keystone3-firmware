# -*- coding: utf-8 -*-
# !/usr/bin/python

import csv
import yaml
import argparse
import subprocess
import glob
import os

compile_command = 'lv_i18n compile -t "*.yml" -o .'

with open("./data.csv", newline="", encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    en = {}
    ru = {}
    cn = {}
    ko = {}
    es = {}

    for row in reader:
        id = row['ID']
        en[id] = row['en']
        ru[id] = row['ru']
        cn[id] = row['cn']
        ko[id] = row['ko']
        es[id] = row['es']

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

compile_command += ' -l en'

print(compile_command)

cmd_result = os.system(compile_command)
if cmd_result != 0:
    exit(cmd_result)