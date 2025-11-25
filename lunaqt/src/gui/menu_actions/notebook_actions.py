"""Notebooks menu action handlers."""

import logging
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow

logger = logging.getLogger(__name__)


def on_new_notebook(window: "QMainWindow") -> None:
    """Handle new notebook action.
    
    Args:
        window: Main window instance
    """
    logger.info("New notebook action triggered")


def on_delete_notebook(window: "QMainWindow") -> None:
    """Handle delete notebook action.
    
    Args:
        window: Main window instance
    """
    logger.info("Delete notebook action triggered")
    if hasattr(window, "delete_active_notebook"):
        window.delete_active_notebook()


def on_select_notebook(window: "QMainWindow") -> None:
    """Handle select notebook action.
    
    Args:
        window: Main window instance
    """
    logger.info("Select notebook action triggered")
