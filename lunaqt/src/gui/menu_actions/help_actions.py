"""Help menu action handlers."""

import logging
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow

logger = logging.getLogger(__name__)


def on_about(window: "QMainWindow") -> None:
    """Handle about action.
    
    Args:
        window: Main window instance
    """
    logger.info("About Luna STEM Notebook action triggered")


def on_help_markdown(window: "QMainWindow") -> None:
    """Handle markdown help action.
    
    Args:
        window: Main window instance
    """
    logger.info("Markdown help action triggered")


def on_help_python(window: "QMainWindow") -> None:
    """Handle Python help action.
    
    Args:
        window: Main window instance
    """
    logger.info("Python help action triggered")


def on_help_cas(window: "QMainWindow") -> None:
    """Handle CAS help action.
    
    Args:
        window: Main window instance
    """
    logger.info("CAS help action triggered")


def on_help_geometry(window: "QMainWindow") -> None:
    """Handle geometry help action.
    
    Args:
        window: Main window instance
    """
    logger.info("Geometry help action triggered")
