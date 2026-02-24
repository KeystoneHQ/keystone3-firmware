#!/usr/bin/env python3
# create_ur_qr.py
# Usage:
# 1) single UR on command line: python create_ur_qr.py "ur:kaspa-pskt/...."
# 2) fragments file: python create_ur_qr.py --file parts.txt
# 3) produce GIF: add --gif out.gif

import argparse
import os
import qrcode
from PIL import Image

parser = argparse.ArgumentParser()
parser.add_argument("ur", nargs="?", help="Single UR string")
parser.add_argument("--file", "-f", help="Text file with one UR fragment per line")
parser.add_argument("--outdir", "-o", default="ur_qr", help="Output dir")
parser.add_argument("--gif", help="Optional output animated gif path")
args = parser.parse_args()

fragments = []
if args.file:
    with open(args.file, "r", encoding="utf-8") as fh:
        fragments = [line.strip() for line in fh if line.strip()]
elif args.ur:
    fragments = [args.ur.strip()]
else:
    parser.error("Provide a UR string or --file")

os.makedirs(args.outdir, exist_ok=True)
images = []
for i, u in enumerate(fragments, start=1):
    img = qrcode.make(u)
    path = os.path.join(args.outdir, f"ur_part_{i:03}.png")
    img.save(path)
    print("Wrote", path)
    images.append(Image.open(path).convert("RGBA"))

if args.gif and images:
    images[0].save(
        args.gif,
        save_all=True,
        append_images=images[1:],
        duration=800,
        loop=0,
        optimize=True,
    )
    print("Wrote animated GIF:", args.gif)