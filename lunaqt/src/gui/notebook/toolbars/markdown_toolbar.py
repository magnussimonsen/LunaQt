"""Toolbar for markdown cells."""

from PySide6.QtCore import Signal
from .base_toolbar import BaseToolbar


class MarkdownToolbar(BaseToolbar):
    """Toolbar for markdown cells with Preview button."""
    
    # Signals
    preview_requested = Signal()
    
    def __init__(self, parent=None) -> None:
        """Initialize markdown toolbar."""
        super().__init__(parent)
        
        # Add Preview button
        self.preview_button = self.add_button("👁 Preview", self._on_preview_clicked)
        
        # Push remaining items to the right
        self.add_stretch()
    
    def _on_preview_clicked(self) -> None:
        """Handle Preview button click."""
        self.preview_requested.emit()
