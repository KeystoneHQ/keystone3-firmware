
# -*- coding: utf-8 -*-
# !/usr/bin/python

import pandas as pd

df = pd.read_csv('data.csv')

start_column = 'ru'
end_column = 'fr'

try:
    start_idx = df.columns.get_loc(start_column)
    end_idx = df.columns.get_loc(end_column) + 1
    en_col = df['en']

    for index, row in df.iloc[:, start_idx:end_idx].iterrows():
        for col in df.columns[start_idx:end_idx]:
            if pd.isna(row[col]):
                raise ValueError(f"data.csv {index} col '{col}' check no translation \r\nen: {en_col[index]}")

except ValueError as e:
    print(f"error: {e}")
else:
    print("check over.")