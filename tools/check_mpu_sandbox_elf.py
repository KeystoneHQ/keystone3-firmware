import argparse
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Section:
    address: int
    size: int

    @property
    def end(self) -> int:
        return self.address + self.size

    def contains(self, address: int) -> bool:
        return self.address <= address < self.end


REQUIRED_SECTIONS = {
    ".freertos_system_calls": 0x800,
    ".sandbox_text": 0x4000,
    ".mpu_sandbox_rw": 0x4000,
    ".mpu_sandbox_stack": 0x1000,
    ".mpu_sandbox_input": 0x100,
}
REQUIRED_SYMBOL_SECTIONS = {
    "g_mpuSandboxIpc": ".privileged_data",
    "g_mpuSandboxInput": ".mpu_sandbox_input",
    "g_mpuSandboxRw": ".mpu_sandbox_rw",
    "mpu_sandbox_validate_ur": ".sandbox_text",
}
ALLOWED_SANDBOX_SYSTEM_CALLS = {"MPU_xTaskGenericNotifyWait"}

DIRECT_TRANSFER_RE = re.compile(
    r"^\s*(?P<source>[0-9a-fA-F]+):.*\b"
    r"(?P<mnemonic>blx?|b(?:eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le)?"
    r"(?:\.[nw])?|cbz|cbnz)\s+"
    r"(?P<target>[0-9a-fA-F]+)\s+<(?P<symbol>[^>]+)>",
    re.MULTILINE,
)
INDIRECT_TRANSFER_RE = re.compile(
    r"^\s*(?P<source>[0-9a-fA-F]+):.*\b"
    r"(?P<mnemonic>blx?|bx)\s+(?P<register>r\d+|ip|lr)\s*$",
    re.MULTILINE,
)


def run_objdump(objdump: str, *arguments: str) -> str:
    return subprocess.run(
        [objdump, *arguments],
        check=True,
        capture_output=True,
        text=True,
    ).stdout


def read_sections(objdump: str, elf: Path) -> dict[str, Section]:
    output = run_objdump(objdump, "-h", str(elf))
    sections: dict[str, Section] = {}
    pattern = re.compile(
        r"^\s*\d+\s+(?P<name>\.\S+)\s+(?P<size>[0-9a-fA-F]+)\s+"
        r"(?P<vma>[0-9a-fA-F]+)\s+",
        re.MULTILINE,
    )
    for match in pattern.finditer(output):
        sections[match.group("name")] = Section(
            address=int(match.group("vma"), 16),
            size=int(match.group("size"), 16),
        )
    return sections


def verify_sections(sections: dict[str, Section]) -> None:
    for name, expected_size in REQUIRED_SECTIONS.items():
        section = sections.get(name)
        if section is None:
            raise RuntimeError(f"missing required ELF section: {name}")
        if section.size != expected_size:
            raise RuntimeError(
                f"{name} size is 0x{section.size:x}, expected 0x{expected_size:x}"
            )
        if section.address % expected_size != 0:
            raise RuntimeError(
                f"{name} address 0x{section.address:x} is not 0x{expected_size:x}-aligned"
            )


LINKER_BOUNDARY_SYMBOLS = {
    "__privileged_functions_start__",
    "__privileged_functions_end__",
    "__syscalls_flash_start__",
    "__syscalls_flash_end__",
    "__syscalls_flash_used_end__",
}


def read_syscall_symbols(objdump: str, elf: Path) -> dict[int, list[str]]:
    output = run_objdump(objdump, "-t", str(elf))
    symbols: dict[int, list[str]] = {}
    for line in output.splitlines():
        parts = line.split()
        if len(parts) < 6 or not re.fullmatch(r"[0-9a-fA-F]+", parts[0]):
            continue
        if parts[-3] not in ("freertos_system_calls", ".freertos_system_calls"):
            continue
        name = parts[-1]
        if name in LINKER_BOUNDARY_SYMBOLS or name.startswith("."):
            continue
        symbols.setdefault(int(parts[0], 16), []).append(name)
    return symbols


def verify_calls(objdump: str, elf: Path, sections: dict[str, Section]) -> None:
    output = run_objdump(objdump, "-d", "-j", ".sandbox_text", str(elf))
    syscall_symbols = read_syscall_symbols(objdump, elf)
    allowed = (sections[".sandbox_text"], sections[".freertos_system_calls"])
    violations = []
    for match in DIRECT_TRANSFER_RE.finditer(output):
        target = int(match.group("target"), 16)
        if not any(section.contains(target) for section in allowed):
            violations.append(
                f"0x{int(match.group('source'), 16):x} transfers via "
                f"{match.group('mnemonic')} to "
                f"0x{target:x} <{match.group('symbol')}>"
            )
        elif sections[".freertos_system_calls"].contains(target):
            names = syscall_symbols.get(target, [])
            symbol = names[0] if names else match.group("symbol").split("+", 1)[0]
            if symbol.endswith("Entry"):
                symbol = symbol[: -len("Entry")]
            if symbol not in ALLOWED_SANDBOX_SYSTEM_CALLS:
                violations.append(
                    f"0x{int(match.group('source'), 16):x} calls disallowed system call "
                    f"0x{target:x} <{symbol}>"
                )
    if violations:
        raise RuntimeError(
            "sandbox code calls outside its code or syscall regions:\n  "
            + "\n  ".join(violations)
        )

    indirect_calls = []
    indirect_tail_calls = []
    for match in INDIRECT_TRANSFER_RE.finditer(output):
        mnemonic = match.group("mnemonic")
        register = match.group("register")
        if mnemonic == "bx" and register == "lr":
            continue
        if mnemonic == "bx":
            indirect_tail_calls.append(
                f"0x{int(match.group('source'), 16):x} branches indirectly via {register}"
            )
        else:
            indirect_calls.append(match.group(0))
    if indirect_tail_calls:
        raise RuntimeError(
            "sandbox contains unchecked indirect tail branch(es):\n  "
            + "\n  ".join(indirect_tail_calls)
        )
    if indirect_calls:
        raise RuntimeError(
            "sandbox contains unchecked indirect call(s):\n  "
            + "\n  ".join(indirect_calls)
        )


def verify_no_writable_globals(objdump: str, elf: Path) -> None:
    output = run_objdump(objdump, "-t", str(elf))
    symbols = {}
    for name in ("__sandbox_writable_start", "__sandbox_writable_end"):
        match = re.search(
            rf"^(?P<address>[0-9a-fA-F]+)\s+\S+.*\s{name}$",
            output,
            re.MULTILINE,
        )
        if match is None:
            raise RuntimeError(f"missing linker guard symbol: {name}")
        symbols[name] = int(match.group("address"), 16)

    if symbols["__sandbox_writable_start"] != symbols["__sandbox_writable_end"]:
        raise RuntimeError("sandbox objects define writable globals outside the sandbox RW region")


def verify_ipc_symbol_placement(
    objdump: str, elf: Path, sections: dict[str, Section]
) -> None:
    output = run_objdump(objdump, "-t", str(elf))
    for symbol, section_name in REQUIRED_SYMBOL_SECTIONS.items():
        match = re.search(
            rf"^(?P<address>[0-9a-fA-F]+)\s+.*\s{re.escape(symbol)}$",
            output,
            re.MULTILINE,
        )
        if match is None:
            raise RuntimeError(f"missing sandbox IPC symbol: {symbol}")
        section = sections.get(section_name)
        if section is None or not section.contains(int(match.group("address"), 16)):
            raise RuntimeError(f"{symbol} is outside {section_name}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Verify Parser Sandbox ELF isolation")
    parser.add_argument("elf", type=Path)
    parser.add_argument("--objdump", default="arm-none-eabi-objdump")
    args = parser.parse_args()

    sections = read_sections(args.objdump, args.elf)
    verify_sections(sections)
    verify_calls(args.objdump, args.elf, sections)
    verify_no_writable_globals(args.objdump, args.elf)
    verify_ipc_symbol_placement(args.objdump, args.elf, sections)
    print("MPU sandbox ELF layout and call closure: PASS")


if __name__ == "__main__":
    main()
