"""File menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def on_new_file(window: "QMainWindow") -> None:
    """Handle new file action.
    
    Args:
        window: Main window instance
    """
    print("New file action triggered")


def on_open_file(window: "QMainWindow") -> None:
    """Handle open file action.
    
    Args:
        window: Main window instance
    """
    print("Open file action triggered")


def on_save_file(window: "QMainWindow") -> None:
    """Handle save file action.
    
    Args:
        window: Main window instance
    """
    print("Save file action triggered")


def on_save_file_as(window: "QMainWindow") -> None:
    """Handle save file as action.
    
    Args:
        window: Main window instance
    """
    print("Save file as action triggered")


def on_export_pdf(window: "QMainWindow") -> None:
    """Handle export as PDF action.
    
    Args:
        window: Main window instance
    """
    print("Export as PDF action triggered")


def on_about(window: "QMainWindow") -> None:
    """Handle about action.
    
    Args:
        window: Main window instance
    """
    print("About Luna STEM Notebook action triggered")


def on_quit(window: "QMainWindow") -> None:
    """Handle quit action.
    
    Args:
        window: Main window instance
    """
    window.close()
