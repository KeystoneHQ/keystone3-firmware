#!/bin/bash

# 遍历 src 目录下的所有 .c 文件
find src -type f -name "*.c" | while read filename
do
    # 使用 sed 命令修改文件
    sed -i '/lv_event_code_t code = lv_event_get_code(e);/,/}/ { /lv_event_code_t code = lv_event_get_code(e);/d; /if (code == LV_EVENT_CLICKED)/d; /}/d }' "$filename"
    echo "Processed: $filename"
done