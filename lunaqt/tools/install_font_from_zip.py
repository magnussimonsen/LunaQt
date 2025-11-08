"""Install font family from a downloaded Google Fonts ZIP.

Usage (PowerShell):
  # 1) Download the family ZIP in your browser from Google Fonts
  # 2) Save as: lunaqt/src/assets/fonts/temp-font-downloads/<Family>.zip
  # 3) Run this script to extract and copy TTFs + license into assets/fonts/<Family>

Example:
  python tools/install_font_from_zip.py "Comic Neue" "lunaqt/src/assets/fonts/temp-font-downloads/ComicNeue.zip"

This keeps network access out of the script and works offline once you have the ZIP.
"""
from __future__ import annotations

import os
import sys
import shutil
import zipfile
from pathlib import Path


def install_font_from_zip(family_name: str, zip_path: Path, target_root: Path) -> None:
    if not zip_path.exists():
        raise FileNotFoundError(f"ZIP not found: {zip_path}")

    extract_dir = target_root / "temp-font-downloads" / f"{family_name.replace(' ', '')}Extract"
    if extract_dir.exists():
        shutil.rmtree(extract_dir)
    extract_dir.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(extract_dir)

    target_dir = target_root / family_name.replace(" ", "")
    # Prefer folder name without spaces for assets path, but the internal family is preserved in TTF
    # If you prefer exact folder name, comment above and use Path(family_name)
    if target_dir.exists():
        shutil.rmtree(target_dir)
    target_dir.mkdir(parents=True, exist_ok=True)

    copied = 0
    for root, _, files in os.walk(extract_dir):
        for fn in files:
            if fn.lower().endswith(".ttf"):
                shutil.copy2(Path(root) / fn, target_dir / fn)
                copied += 1
    ofl_src = extract_dir / "OFL.txt"
    if ofl_src.exists():
        shutil.copy2(ofl_src, target_dir / "OFL.txt")

    print(f"Installed {copied} TTF files to {target_dir}")


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print("Usage: python tools/install_font_from_zip.py <Family Name> <ZIP Path>")
        return 2
    family = argv[1]
    zip_path = Path(argv[2]).resolve()
    target_root = Path(__file__).resolve().parents[1] / "src" / "assets" / "fonts"
    install_font_from_zip(family, zip_path, target_root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
