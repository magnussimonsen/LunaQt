"""View menu action handlers."""

import logging
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..main_window import MainWindow

logger = logging.getLogger(__name__)


def on_normal_view(window: "MainWindow") -> None:
    """Handle normal view action.
    
    Args:
        window: Main window instance
    """
    logger.info("Normal web view action triggered")


def on_a4_view(window: "MainWindow") -> None:
    """Handle A4 view action.
    
    Args:
        window: Main window instance
    """
    logger.info("A4 paper view action triggered")
