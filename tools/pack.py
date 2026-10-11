# Packs the JEI source folder into @JEI/addons/JEI.pbo (unbinarized, uncompressed).
# Author: vyrriox
#
# Usage: python tools/pack.py

import hashlib
import shutil
import struct
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "JEI"
OUTPUT_DIR = ROOT / "@JEI" / "addons"
OUTPUT = OUTPUT_DIR / "JEI.pbo"
SKIPPED = {"$PBOPREFIX$"}


def entry_header(name, size, timestamp, method=0, original=0):
    return name.encode("utf-8") + b"\0" + struct.pack("<5I", method, original, 0, timestamp, size)


def main():
    prefix = (SOURCE / "$PBOPREFIX$").read_text(encoding="utf-8").strip()
    files = sorted(p for p in SOURCE.rglob("*") if p.is_file() and p.name not in SKIPPED)

    body = bytearray()
    # Header extension holding the prefix.
    body += entry_header("", 0, 0, method=0x56657273)
    body += b"prefix\0" + prefix.encode("utf-8") + b"\0\0"

    data = bytearray()
    for path in files:
        content = path.read_bytes()
        name = str(path.relative_to(SOURCE)).replace("/", "\\")
        body += entry_header(name, len(content), int(path.stat().st_mtime))
        data += content

    body += entry_header("", 0, 0)
    body += data
    digest = hashlib.sha1(body).digest()

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_bytes(bytes(body) + b"\0" + digest)
    shutil.copy2(ROOT / "mod.cpp", ROOT / "@JEI" / "mod.cpp")
    print(f"{OUTPUT} ({len(files)} files, {OUTPUT.stat().st_size} bytes) {time.strftime('%H:%M:%S')}")


if __name__ == "__main__":
    main()
