"""Help menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def on_about(window: "QMainWindow") -> None:
    """Handle about action.
    
    Args:
        window: Main window instance
    """
    print("About Luna STEM Notebook action triggered")


def on_help_markdown(window: "QMainWindow") -> None:
    """Handle markdown help action.
    
    Args:
        window: Main window instance
    """
    print("Markdown help action triggered")


def on_help_python(window: "QMainWindow") -> None:
    """Handle Python help action.
    
    Args:
        window: Main window instance
    """
    print("Python help action triggered")


def on_help_cas(window: "QMainWindow") -> None:
    """Handle CAS help action.
    
    Args:
        window: Main window instance
    """
    print("CAS help action triggered")


def on_help_geometry(window: "QMainWindow") -> None:
    """Handle geometry help action.
    
    Args:
        window: Main window instance
    """
    print("Geometry help action triggered")
