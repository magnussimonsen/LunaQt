"""Centralized helpers to update GUI styles when theme changes.

This keeps all widget-specific restyling in one place, called after the
core ThemeManager applies the application stylesheet. It updates elements
that rely on dynamic inline styles (e.g., the Settings button) and any
accented widgets.
"""
from __future__ import annotations

from typing import Optional
from PySide6.QtWidgets import QMainWindow

from ...themes.colors import get_color
from ...constants.types import ThemeMode
from ..button_styles import get_button_stylesheet


def apply_theme_styles(window: QMainWindow, theme: ThemeMode) -> None:
    """Apply theme-dependent styles to specific widgets.

    Should be called right after ThemeManager.apply_theme(theme) so that
    component-specific styles remain in sync with the current theme.
    """
    # Header accent color (kept here to centralize, duplicates ThemeManager safety)
    if hasattr(window, "header_label") and window.header_label is not None:
        accent = get_color("primary_accent_color", theme)
        window.header_label.setStyleSheet(f"color: {accent};")

    # Settings button dynamic style based on visibility (selected state)
    if hasattr(window, "settings_button") and window.settings_button is not None:
        is_selected = False
        if hasattr(window, "settings_dock") and window.settings_dock is not None:
            is_selected = window.settings_dock.isVisible()
        state = "selected" if is_selected else "normal"
        window.settings_button.setStyleSheet(get_button_stylesheet(theme, state))
