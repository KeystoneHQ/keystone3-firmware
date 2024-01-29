import csv
import yaml

with open("./data.csv", newline="", encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    en = {}
    ru = {}
    cn = {}

    for row in reader:
        id = row['ID(0.0.1)']
        en[id]=row['en']
        ru[id]=row['Russian']
        cn[id]=row['cn']

    with open("../en.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'en': en}, f, default_flow_style=False)

    with open("../ru.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'ru': ru}, f, allow_unicode=True, default_flow_style=False)
        
    with open("../zh-CN:.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'zh-CN:': cn}, f, allow_unicode=True, default_flow_style=False)