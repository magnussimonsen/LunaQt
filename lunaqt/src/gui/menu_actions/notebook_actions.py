"""Notebooks menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def on_new_notebook(window: "QMainWindow") -> None:
    """Handle new notebook action.
    
    Args:
        window: Main window instance
    """
    print("New notebook action triggered")


def on_delete_notebook(window: "QMainWindow") -> None:
    """Handle delete notebook action.
    
    Args:
        window: Main window instance
    """
    print("Delete notebook action triggered")


def on_select_notebook(window: "QMainWindow") -> None:
    """Handle select notebook action.
    
    Args:
        window: Main window instance
    """
    print("Select notebook action triggered")
