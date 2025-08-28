#!/usr/bin/env bash
set -euo pipefail

SRC="${1:-build/keystone3.bin}"

# 目标挂载点，改成你实际的 U 盘挂载目录
DEST="/Volumes/NO NAME"

# 校验
if [ ! -f "$SRC" ]; then
  echo "❌ 源文件不存在：$SRC"
  exit 1
fi
if [ ! -d "$DEST" ]; then
  echo "❌ 挂载点不存在：$DEST 请确认 U 盘已挂载"
  exit 1
fi

# 执行拷贝
echo "📋 拷贝 $SRC → $DEST/pillar.bin"
cp "$SRC" "$DEST/pillar.bin"

echo "✅ 完成拷贝！"