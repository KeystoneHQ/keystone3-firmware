#!/usr/bin/env python3
import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict


SECTION_PREFIXES = (
    ".text",
    ".rodata",
    ".init_array",
    ".fini_array",
    ".ctors",
    ".dtors",
    ".ARM.extab",
    ".ARM.exidx",
    ".gcc_except_table",
    ".eh_frame",
)


def is_flash_section(sec: str) -> bool:
    return any(sec == p or sec.startswith(p + ".") for p in SECTION_PREFIXES)


def norm_path(s: str) -> str:
    return s.replace("\\", "/")


def module_of(obj: str) -> str:
    p = norm_path(obj)
    if ".a(" in p:
        return p.split(".a(", 1)[0] + ".a"
    if "/external/" in p:
        tail = p.split("/external/", 1)[1]
        return "external/" + tail.split("/", 1)[0]
    if "/src/" in p:
        tail = p.split("/src/", 1)[1]
        return "src/" + tail.split("/", 1)[0]
    if "/rust/" in p:
        tail = p.split("/rust/", 1)[1]
        return "rust/" + tail.split("/", 1)[0]
    return p.split("/", 1)[0]


def parse_map(map_path: str):
    line_re = re.compile(r"^\s+(\.\S+)\s+0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)\s+(.+)$")
    by_obj = defaultdict(int)
    by_mod = defaultdict(int)
    by_arc = defaultdict(int)
    total = 0
    in_memory_map = False
    with open(map_path, "r", encoding="utf-8", errors="ignore") as f:
        for raw in f:
            if not in_memory_map:
                if raw.strip() == "Linker script and memory map":
                    in_memory_map = True
                continue
            m = line_re.match(raw)
            if not m:
                continue
            sec, size_hex, owner = m.groups()
            if not is_flash_section(sec):
                continue
            if owner.startswith("*fill*"):
                continue
            size = int(size_hex, 16)
            if size == 0:
                continue
            owner = owner.strip()
            total += size
            by_obj[owner] += size
            by_mod[module_of(owner)] += size
            if ".a(" in owner:
                by_arc[owner.split(".a(", 1)[0] + ".a"] += size
    return total, by_obj, by_mod, by_arc


def find_nm():
    env_nm = os.environ.get("NM")
    if env_nm:
        return env_nm
    for cand in ("arm-none-eabi-nm", "nm"):
        try:
            subprocess.run([cand, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            return cand
        except Exception:
            pass
    return None


def top_symbols(elf_path: str, top_n: int):
    nm = find_nm()
    if nm is None:
        return []
    try:
        proc = subprocess.run(
            [nm, "-S", "--size-sort", elf_path],
            capture_output=True,
            text=True,
            check=True,
        )
    except Exception:
        return []
    # format: <addr> <size> <type> <name>
    sym_re = re.compile(r"^\s*[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+([A-Za-z])\s+(.+)$")
    rows = []
    for line in proc.stdout.splitlines():
        m = sym_re.match(line)
        if not m:
            continue
        size_hex, typ, name = m.groups()
        # T/t: text, R/r: rodata
        if typ not in ("T", "t", "R", "r"):
            continue
        size = int(size_hex, 16)
        if size <= 0:
            continue
        rows.append((size, typ, name))
    rows.sort(key=lambda x: x[0], reverse=True)
    return rows[:top_n]


def print_top(title: str, items, top_n: int):
    print(f"\n{title}")
    for i, (name, size) in enumerate(sorted(items.items(), key=lambda kv: kv[1], reverse=True)[:top_n], start=1):
        print(f"{i:>2}. {size:>8} B  {size/1024:>7.2f} KB  {name}")


def main():
    ap = argparse.ArgumentParser(description="Analyze flash-heavy objects/modules/symbols from map/elf.")
    ap.add_argument("--map", dest="map_path", default="build/mh1903.map", help="Path to linker .map file")
    ap.add_argument("--elf", dest="elf_path", default="build/mh1903.elf", help="Path to ELF for symbol-level analysis")
    ap.add_argument("--top", dest="top_n", type=int, default=30, help="Top N entries per table")
    args = ap.parse_args()

    if not os.path.isfile(args.map_path):
        print(f"map file not found: {args.map_path}", file=sys.stderr)
        return 2

    total, by_obj, by_mod, by_arc = parse_map(args.map_path)
    print("Flash Usage Summary")
    print(f"Total (parsed flash sections): {total} B ({total/1024:.2f} KB)")
    print(f"Map file: {args.map_path}")

    print_top("Top Modules", by_mod, args.top_n)
    print_top("Top Objects/Archive Members", by_obj, args.top_n)
    if by_arc:
        print_top("Top Static Archives", by_arc, args.top_n)

    if os.path.isfile(args.elf_path):
        syms = top_symbols(args.elf_path, args.top_n)
        if syms:
            print("\nTop Symbols (from nm, .text/.rodata)")
            for i, (size, typ, name) in enumerate(syms, start=1):
                print(f"{i:>2}. {size:>8} B  {size/1024:>7.2f} KB  [{typ}] {name}")
        else:
            print("\nTop Symbols: unavailable (nm not found or parse failed)")
    else:
        print(f"\nELF not found, skip symbol analysis: {args.elf_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
