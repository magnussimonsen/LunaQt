"""View menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..main_window import MainWindow


def on_normal_view(window: "MainWindow") -> None:
    """Handle normal view action.
    
    Args:
        window: Main window instance
    """
    print("Normal web view action triggered")


def on_a4_view(window: "MainWindow") -> None:
    """Handle A4 view action.
    
    Args:
        window: Main window instance
    """
    print("A4 paper view action triggered")
