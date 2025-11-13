"""Empty toolbar shown when no cell is selected."""

from PySide6.QtCore import Qt
from .base_toolbar import BaseToolbar


class EmptyToolbar(BaseToolbar):
    """Toolbar shown when no cell is selected."""
    
    def __init__(self, parent=None) -> None:
        """Initialize empty toolbar."""
        super().__init__(parent)
        
        # Add "No cell selected" label
        label = self.add_label("No cell selected")
        label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.add_stretch()
