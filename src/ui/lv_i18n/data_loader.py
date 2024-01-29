import csv
import yaml
import argparse
import subprocess
import glob

parser = argparse.ArgumentParser(description='Convert CSV to YAML')
parser.add_argument('--ru', action='store_true', help='Generate Russian translations')
parser.add_argument('--zh', action='store_true', help='Generate Chinese (Simplified) translations')
args = parser.parse_args()

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

# 总是生成英文 YAML
with open("./en.yml", 'w', encoding='utf-8') as f:
    yaml.dump({'en': en}, f, default_flow_style=False)

# 根据参数生成俄文和中文 YAML
if args.ru:
    with open("./ru.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'ru': ru}, f, allow_unicode=True, default_flow_style=False)

if args.zh:
    with open("./zh-CN.yml", 'w', encoding='utf-8') as f:
        yaml.dump({'zh-CN': cn}, f, allow_unicode=True, default_flow_style=False)

# 获取所有的 .yml 文件
yml_files = glob.glob('*.yml')

# 构建命令
cmd = ['lv_i18n', 'compile', '-t'] + yml_files + ['-o', '.', '-l', 'zh-CN', '-l', 'en']

print(cmd)

# 执行命令
result = subprocess.run(cmd)

# 检查是否执行成功
if result.returncode == 0:
    print("命令执行成功:")
    print(result.stdout)
else:
    print("命令执行出错:")
    print(result.stderr)