import csv
import yaml

with open("./data.csv", newline="") as csvfile:
    reader = csv.DictReader(csvfile)
    en = {}

    for row in reader:
        id = row['ID(0.0.1)']
        text = row['en']
        en[id]=text

    with open("en.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'en': en}, f, default_flow_style=False)