import pandas as pd

# 加载 CSV 文件
df = pd.read_csv('data.csv')

# 确保 'en' 列存在
if 'en' not in df.columns:
    raise ValueError("There is no 'en' column in the dataframe.")

# 查看数据的前几行，确认数据的一般形态
# print(df.head())

# 查看 'en' 列中的唯一值及其出现次数
# print(df['en'].value_counts())

# 移除 'en' 列的可能空格（如果需要）
df['en'] = df['en'].str.strip()

# 筛选出 'en' 列中完全相同的项
duplicates = df[df.duplicated('en', keep=False)]

# 如果你想看具体哪些值是重复的
duplicate_values = duplicates['ID'].unique()

# 显示重复的行
print(duplicates)

# 显示重复的值
print(duplicate_values)