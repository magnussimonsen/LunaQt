"""Centralized application icon utilities.

This module provides helpers to:
 1. Resolve icon file paths (dev vs PyInstaller frozen build)
 2. Construct a QIcon for the window
 3. Create a header widget (icon + title label) for reuse

Edit ICON_BASE_NAME if you change your primary icon filename once; all uses
will pick up the change.
"""
from __future__ import annotations

import sys
from pathlib import Path
from typing import Optional
from PySide6.QtGui import QIcon, QPixmap
from PySide6.QtWidgets import QLabel, QWidget, QHBoxLayout
from PySide6.QtCore import Qt

# Base filename (without extension preference). You can change this once here.
# ICON_BASE_NAME = "icon"  # Change here to switch the global app icon base name
ICON_BASE_NAME = "normal_distribution_3d"  # Change here to switch the global app icon base name
# Default header icon height (kept modest to avoid layout pressure)
DEFAULT_HEADER_ICON_HEIGHT = 64



def _base_dir() -> Path:
    """Return the base directory where icons are located (handles frozen)."""
    if hasattr(sys, "_MEIPASS"):
        return Path(sys._MEIPASS) / 'src' / 'icons'
    # This file: src/styling/icon_utils.py -> go up two to src, then icons
    return Path(__file__).resolve().parents[1] / 'icons'


def get_icon_path(icon_name: str) -> Path:
    """Get the absolute path to an icon file.
    
    Args:
        icon_name: Name of the icon file (e.g., 'normal_distribution_3d.png')
    
    Returns:
        Absolute path to the icon file
    """
    return _base_dir() / icon_name


def resolve_icon(preferred: str = ICON_BASE_NAME) -> dict[str, Path]:
    """Resolve common icon variant paths (ico, png) if they exist.

    Returns a dict with keys that exist on disk: {'ico': Path, 'png': Path}
    """
    base = _base_dir()
    variants: dict[str, Path] = {}
    ico = base / f"{preferred}.ico"
    png = base / f"{preferred}.png"
    if ico.exists():
        variants['ico'] = ico
    if png.exists():
        variants['png'] = png
    return variants


def get_app_icon() -> Optional[QIcon]:
    """Build a QIcon for the main window (prefers .ico on Windows)."""
    variants = resolve_icon()
    chosen: Optional[Path] = None
    # Prefer ICO if present (best for Windows taskbar); else PNG
    if 'ico' in variants:
        chosen = variants['ico']
    elif 'png' in variants:
        chosen = variants['png']
    if chosen is None:
        return None
    return QIcon(str(chosen))


def create_header_widget(title: str, icon_height: int | None = None) -> tuple[QWidget, QLabel, QLabel]:
    """Create a header widget containing the app icon and a title label.

    Returns (container_widget, title_label, icon_label) so caller can further style the labels.
    """
    container = QWidget()
    layout = QHBoxLayout()
    layout.setContentsMargins(0, 0, 0, 0)
    layout.setSpacing(8)
    layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
    container.setLayout(layout)

    icon_label = QLabel()
    variants = resolve_icon()
    pm: Optional[QPixmap] = None
    h = icon_height if icon_height is not None else DEFAULT_HEADER_ICON_HEIGHT
    if 'png' in variants:
        pix = QPixmap(str(variants['png']))
        if not pix.isNull():
            pm = pix.scaledToHeight(h, Qt.TransformationMode.SmoothTransformation)
    elif 'ico' in variants:
        pix = QPixmap(str(variants['ico']))
        if not pix.isNull():
            pm = pix.scaledToHeight(h, Qt.TransformationMode.SmoothTransformation)
    if pm is not None:
        icon_label.setPixmap(pm)

    title_label = QLabel(title)
    title_label.setAlignment(Qt.AlignmentFlag.AlignVCenter)

    layout.addWidget(icon_label)
    layout.addWidget(title_label)
    return container, title_label, icon_label


def resize_header_icon(icon_label: QLabel, new_height: int) -> None:
    """Resize an existing header icon QLabel to a new height (smooth scaling)."""
    pm = icon_label.pixmap()
    if pm is None or pm.isNull():
        return
    scaled = pm.scaledToHeight(new_height, Qt.TransformationMode.SmoothTransformation)
    icon_label.setPixmap(scaled)
