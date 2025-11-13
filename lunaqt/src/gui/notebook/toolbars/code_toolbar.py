"""Toolbar for code cells."""

from PySide6.QtCore import Signal
from .base_toolbar import BaseToolbar


class CodeToolbar(BaseToolbar):
    """Toolbar for code cells with Run and Reset buttons."""
    
    # Signals
    run_requested = Signal()
    reset_requested = Signal()
    
    def __init__(self, parent=None) -> None:
        """Initialize code toolbar."""
        super().__init__(parent)
        
        # Add Run button
        self.run_button = self.add_button("▶ Run", self._on_run_clicked)
        
        # Add Reset button
        self.reset_button = self.add_button("↻ Reset", self._on_reset_clicked)
        
        # Push remaining items to the right
        self.add_stretch()
    
    def _on_run_clicked(self) -> None:
        """Handle Run button click."""
        self.run_requested.emit()
    
    def _on_reset_clicked(self) -> None:
        """Handle Reset button click."""
        self.reset_requested.emit()
