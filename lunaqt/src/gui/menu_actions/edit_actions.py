"""Edit menu action handlers."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def on_move_cell_up(window: "QMainWindow") -> None:
    """Handle move cell up action.
    
    Args:
        window: Main window instance
    """
    print("Move cell up action triggered")


def on_move_cell_down(window: "QMainWindow") -> None:
    """Handle move cell down action.
    
    Args:
        window: Main window instance
    """
    print("Move cell down action triggered")


def on_delete_cell(window: "QMainWindow") -> None:
    """Handle delete cell action.
    
    Args:
        window: Main window instance
    """
    print("Delete cell action triggered")


def on_insert_text_cell(window: "QMainWindow") -> None:
    """Handle insert text cell action.
    
    Args:
        window: Main window instance
    """
    print("Insert text cell action triggered")


def on_insert_cas_cell(window: "QMainWindow") -> None:
    """Handle insert CAS cell action.
    
    Args:
        window: Main window instance
    """
    print("Insert CAS cell action triggered")


def on_insert_python_cell(window: "QMainWindow") -> None:
    """Handle insert Python cell action.
    
    Args:
        window: Main window instance
    """
    print("Insert Python cell action triggered")
