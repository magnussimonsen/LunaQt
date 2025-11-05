#!/usr/bin/env python3
"""
Generate simple LunaQt icons:
COMMAND WIN: python.exe generate_lunaqt_icons.py --png-scale 0.15 --ico-scale 0.15 --font-index 6 --png-offset 0.06 --ico-offset 0.06
COMMAND LINUX: python3 generate_lunaqt_icons.py --png-scale 0.15 --ico-scale 0.15 --font-index 6  --png-offset 0.06 --ico-offset 0.06
-BG_COLOR background with centered white text "LUNA STEM NOTEBOOK"
- Outputs:
  - src/icons/icon.png (256x256)
  - src/icons/icon.ico (multi-size: 16, 32, 48, 64, 128, 256)
"""
from pathlib import Path
from typing import List, Tuple, Optional
import argparse

# Lazy import Pillow with good error message
try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError as e:
    raise SystemExit("Pillow is required. Install with: pip install Pillow")

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "src" / "icons"
PNG_PATH = OUT_DIR / "icon.png"
ICO_PATH = OUT_DIR / "icon.ico"

# BG_COLOR = (33, 105, 202, 255)  # Luna blue
BG_COLOR = (76, 175, 80, 255)  # Luna calm forest green
# FG_COLOR = (255, 255, 255, 255)  # white
FG_COLOR = (0, 0, 0, 255)  # Black 
TEXT = "LUNA\nSTEM \nNOTEBOOK"


def _font_candidates(target_px: int) -> List[Tuple[str, int]]:
    """Return an ordered list of font candidate paths or names with target size.

    Order favors bold, legible UI fonts commonly available on Windows, Linux, and macOS.
    """
    repo_font = (ROOT / "assets" / "fonts" / "LunaQt.ttf")
    candidates: List[Tuple[str, int]] = []

    # Project-provided font (if you drop one in assets/fonts/LunaQt.ttf)
    candidates.append((str(repo_font), target_px))

    # Windows explicit file paths (most reliable)
    win_files = [
        r"C:\\Windows\\Fonts\\segoeuib.ttf",   # Segoe UI Bold
        r"C:\\Windows\\Fonts\\segoeui.ttf",    # Segoe UI
        r"C:\\Windows\\Fonts\\arialbd.ttf",    # Arial Bold
        r"C:\\Windows\\Fonts\\arial.ttf",      # Arial
        r"C:\\Windows\\Fonts\\verdanab.ttf",   # Verdana Bold
        r"C:\\Windows\\Fonts\\verdana.ttf",    # Verdana
        r"C:\\Windows\\Fonts\\tahoma.ttf",     # Tahoma
        r"C:\\Windows\\Fonts\\trebucbd.ttf",   # Trebuchet MS Bold
        r"C:\\Windows\\Fonts\\trebuc.ttf",     # Trebuchet MS
        r"C:\\Windows\\Fonts\\calibrib.ttf",   # Calibri Bold
        r"C:\\Windows\\Fonts\\calibri.ttf",    # Calibri
        r"C:\\Windows\\Fonts\\consola.ttf",    # Consolas (fallback, mono)
    ]
    candidates.extend([(p, target_px) for p in win_files])

    # Pretty names (font manager resolution may work)
    pretty_names = [
        "Segoe UI Bold.ttf",
        "Segoe UI.ttf",
        "Arial Bold.ttf",
        "Arial.ttf",
        "Verdana Bold.ttf",
        "Verdana.ttf",
        "Tahoma.ttf",
        "Trebuchet MS Bold.ttf",
        "Trebuchet MS.ttf",
        "Calibri Bold.ttf",
        "Calibri.ttf",
        "Consolas.ttf",
    ]
    candidates.extend([(n, target_px) for n in pretty_names])

    # Linux
    linux_paths = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    ]
    candidates.extend([(p, target_px) for p in linux_paths])

    # macOS (some are .ttc collections; Pillow can handle many .ttc)
    mac_paths = [
        "/System/Library/Fonts/SFNS.ttf",  # San Francisco (older versions)
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Verdana Bold.ttf",
        "/System/Library/Fonts/Supplemental/Verdana.ttf",
        "/Library/Fonts/Arial Bold.ttf",
        "/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Verdana Bold.ttf",
        "/Library/Fonts/Verdana.ttf",
        "/Library/Fonts/Tahoma.ttf",
    ]
    candidates.extend([(p, target_px) for p in mac_paths])

    return candidates


def _load_font(target_px: int, font_index: Optional[int] = None, font_path: Optional[str] = None) -> ImageFont.FreeTypeFont:
    """Load a font with priority: explicit font_path > indexed candidate > fallbacks > default.

    Args:
        target_px: Desired font size in pixels.
        font_index: Optional index into the candidate list to try first.
        font_path: Optional absolute or relative path to a .ttf/.otf font file.
    """
    # 1) Try explicit font_path if provided
    if font_path:
        try:
            return ImageFont.truetype(font_path, size=target_px)
        except Exception:
            pass  # fall back to candidates

    candidates = _font_candidates(target_px)

    # 2) Try preferred candidate index first (if valid)
    if font_index is not None and 0 <= font_index < len(candidates):
        name, size = candidates[font_index]
        try:
            return ImageFont.truetype(name, size=size)
        except Exception:
            pass  # move on to the rest

    # 3) Try each candidate in order
    for name, size in candidates:
        try:
            return ImageFont.truetype(name, size=size)
        except Exception:
            continue

    # 4) Fallback - will be smaller, but always available
    return ImageFont.load_default()


def _draw_text_centered(draw: ImageDraw.ImageDraw, box: Tuple[int, int, int, int], text: str, font: ImageFont.ImageFont, fill, y_offset_frac: float = 0.0):
    # Pillow >= 8: use textbbox for accurate metrics
    left, top, right, bottom = draw.textbbox((0, 0), text, font=font)
    w, h = right - left, bottom - top
    box_w = box[2] - box[0]
    box_h = box[3] - box[1]
    x = box[0] + (box_w - w) // 2
    # Positive offset moves text UP by that fraction of the box height
    y = box[1] + (box_h - h) // 2 - int(y_offset_frac * box_h)
    draw.text((x, y), text, font=font, fill=fill)


def make_png(size: int = 256, scale: float = 0.42, font_index: Optional[int] = None, font_path: Optional[str] = None, y_offset_frac: float = 0.0):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    img = Image.new("RGBA", (size, size), BG_COLOR)
    draw = ImageDraw.Draw(img)

    # Font size is a fraction of canvas height
    font_size = max(8, int(size * scale))
    font = _load_font(font_size, font_index=font_index, font_path=font_path)

    # Optional rounded rectangle background (subtle)
    radius = int(size * 0.18)
    try:
        draw.rounded_rectangle([(0, 0), (size - 1, size - 1)], radius=radius, fill=BG_COLOR)
    except Exception:
        # rounded_rectangle may not exist on very old Pillow; ignore
        pass

    _draw_text_centered(draw, (0, 0, size, size), TEXT, font, FG_COLOR, y_offset_frac=y_offset_frac)
    img.save(PNG_PATH)
    print(f"Wrote {PNG_PATH}")


def make_ico(scale: float = 0.6, font_index: Optional[int] = None, font_path: Optional[str] = None, y_offset_frac: float = 0.0):
    """
    Create a multi-resolution ICO from a single high-res base image.
    Pillow's ICO saver doesn't support save_all for multiple custom frames
    in some versions; using a single 256x256 base with the requested scale
    reliably produces all sizes via downscaling while honoring scale.
    """
    sizes = [16, 32, 48, 64, 128, 256]
    base_size = 256
    img = Image.new("RGBA", (base_size, base_size), BG_COLOR)
    d = ImageDraw.Draw(img)
    font_size = max(8, int(base_size * scale))
    font = _load_font(font_size, font_index=font_index, font_path=font_path)
    _draw_text_centered(d, (0, 0, base_size, base_size), TEXT, font, FG_COLOR, y_offset_frac=y_offset_frac)

    # Save the base while requesting multiple sizes; ICO plugin will downscale
    img.save(ICO_PATH, format="ICO", sizes=[(s, s) for s in sizes])
    print(f"Wrote {ICO_PATH}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate LunaQt icons (PNG and ICO)")
    parser.add_argument("--png-size", type=int, default=256, help="PNG canvas size (default: 256)")
    parser.add_argument("--png-scale", type=float, default=0.42, help="Text scale for PNG (fraction of size, default: 0.42)")
    parser.add_argument("--ico-scale", type=float, default=0.6, help="Text scale for ICO (fraction of each frame, default: 0.6)")
    parser.add_argument("--font-index", type=int, default=None, help="Preferred index into built-in font candidates (0-based)")
    parser.add_argument("--font-path", type=str, default=None, help="Explicit path to a .ttf/.otf font file to use")
    parser.add_argument("--png-offset", type=float, default=0.0, help="Vertical offset for PNG as fraction of height; positive moves up (e.g., 0.06)")
    parser.add_argument("--ico-offset", type=float, default=0.0, help="Vertical offset for ICO base render; positive moves up")
    parser.add_argument("--list-fonts", action="store_true", help="List candidate fonts with indices and exit")
    args = parser.parse_args()

    if args.list_fonts:
        cands = _font_candidates(64)
        print("Font candidates (index: path-or-name) — 'exists' means file present on this system:")
        for i, (name, _sz) in enumerate(cands):
            exists = Path(name).exists()
            print(f"  {i:2d}: {name} {'[exists]' if exists else ''}")
        raise SystemExit(0)

    make_png(size=args.png_size, scale=args.png_scale, font_index=args.font_index, font_path=args.font_path, y_offset_frac=args.png_offset)
    make_ico(scale=args.ico_scale, font_index=args.font_index, font_path=args.font_path, y_offset_frac=args.ico_offset)
    print("Done generating LunaQt icons.")
