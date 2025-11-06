"""Settings menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def on_font_size(window: "QMainWindow") -> None:
    """Handle font size action.
    
    Args:
        window: Main window instance
    """
    print("Font size action triggered")


def on_font_family(window: "QMainWindow") -> None:
    """Handle font family action.
    
    Args:
        window: Main window instance
    """
    print("Font family action triggered")


def on_precision(window: "QMainWindow") -> None:
    """Handle precision action.
    
    Args:
        window: Main window instance
    """
    print("Precision action triggered")
