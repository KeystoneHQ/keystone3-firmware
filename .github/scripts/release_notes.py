"""Render a draft from the tagged version's English changelog and release checksums."""

import json
import re
import sys
from pathlib import Path


def release_body(version):
    changelog = Path("CHANGELOG.md").read_text()
    sections = re.split(r"^##[ \t]+", changelog, flags=re.MULTILINE)[1:]
    notes = ""
    for section in sections:
        heading, _, content = section.partition("\n")
        if re.fullmatch(re.escape(version) + r"(?:\s*\([^)]*\))?\s*", heading):
            notes = content.strip()
            break

    variants = json.loads(Path(".releases").read_text())[version]
    labels = {"web3": "Web3", "btc_only": "BTC Only", "cypherpunk": "Cypherpunk"}
    sections = ["## Release Notes", notes or f"Keystone3 firmware {version}."]
    for title, field in [("Firmware", "firmware_sha256"), ("Source Code", "source_code_sha256")]:
        sections.append(f"## {title} SHA-256 Checksum")
        if field == "firmware_sha256":
            sections.append("SHA-256 of the signed `keystone3.bin` for each firmware variant.")
        for variant, label in labels.items():
            if variant in variants:
                sections.extend([f"**{label}**", f"```text\n{variants[variant][field]}\n```"])

    sections.extend([
        "[Download firmware](https://keyst.one/firmware) · "
        "[Verify on your device](https://guide.keyst.one/docs/verify-checksum)",
        f"[Build and verification guide](https://github.com/KeystoneHQ/keystone3-firmware/blob/{version}/docs/verify.md)",
    ])
    return "\n\n".join(sections)


if __name__ == "__main__":
    print(release_body(sys.argv[1]))
