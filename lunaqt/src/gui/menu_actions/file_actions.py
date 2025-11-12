"""File menu action handlers."""

import logging
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow

logger = logging.getLogger(__name__)


def on_new_file(window: "QMainWindow") -> None:
    """Handle new file action.
    
    Args:
        window: Main window instance
    """
    logger.info("New file action triggered")


def on_open_file(window: "QMainWindow") -> None:
    """Handle open file action.
    
    Args:
        window: Main window instance
    """
    logger.info("Open file action triggered")


def on_save_file(window: "QMainWindow") -> None:
    """Handle save file action.
    
    Args:
        window: Main window instance
    """
    logger.info("Save file action triggered")


def on_save_file_as(window: "QMainWindow") -> None:
    """Handle save file as action.
    
    Args:
        window: Main window instance
    """
    logger.info("Save file as action triggered")


def on_export_pdf(window: "QMainWindow") -> None:
    """Handle export as PDF action.
    
    Args:
        window: Main window instance
    """
    logger.info("Export as PDF action triggered")


def on_about(window: "QMainWindow") -> None:
    """Handle about action.
    
    Args:
        window: Main window instance
    """
    logger.info("About Luna STEM Notebook action triggered")


def on_quit(window: "QMainWindow") -> None:
    """Handle quit action.
    
    Args:
        window: Main window instance
    """
    window.close()
