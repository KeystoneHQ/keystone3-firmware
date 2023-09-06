## Keystone 3 text generator


## requirements
`Python3.11`, `Node 16`

`npm install -g lv_i18n`
### How to use

> pip3 install -r requirements.txt

### HOW
1. Get the csv from google sheet, ask PM to get the link.
2. Download the sheet as csv, rename it to `data.csv` and copy to this folder.
3. Run `python3 data_loader.py` to generate the `en.yml` and other language file.
4. update the content of `.yml`s to the upper folder.
5. run `lv_i18n compile -t en.yml -o .` to update code.
