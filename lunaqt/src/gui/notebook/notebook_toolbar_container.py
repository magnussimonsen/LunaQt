"""Reactive toolbar container that switches content based on cell selection."""

from PySide6.QtWidgets import QWidget, QVBoxLayout, QStackedWidget
from PySide6.QtCore import Signal
from PySide6.QtGui import QFont

from .toolbars.empty_toolbar import EmptyToolbar
from .toolbars.code_toolbar import CodeToolbar
from .toolbars.markdown_toolbar import MarkdownToolbar
from ...core.font_service import get_font_service


class NotebookToolbarContainer(QWidget):
    """Container widget that displays different toolbars based on cell type.
    
    Manages a stack of toolbars and switches between them reactively.
    """
    
    # Forward signals from child toolbars
    run_code_requested = Signal()
    reset_code_requested = Signal()
    preview_markdown_requested = Signal()
    
    def __init__(self, parent: QWidget | None = None) -> None:
        """Initialize toolbar container.
        
        Args:
            parent: Parent widget
        """
        super().__init__(parent)
        self.setObjectName("NotebookToolbarContainer")
        self._setup_ui()
        
        # Subscribe to font service for UI font changes
        self._font_service = get_font_service()
        self._font_service.uiFontChanged.connect(self._on_ui_font_changed)
        
        # Apply current UI font
        family, size = self._font_service.get_ui_font()
        if family and size:
            self._apply_font(family, size)
    
    def _setup_ui(self) -> None:
        """Set up the UI with stacked toolbars."""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        
        # Stacked widget to hold different toolbars
        self._stack = QStackedWidget()
        layout.addWidget(self._stack)
        
        # Create all toolbar variants
        self._empty_toolbar = EmptyToolbar()
        self._code_toolbar = CodeToolbar()
        self._markdown_toolbar = MarkdownToolbar()
        
        # Connect toolbar signals
        self._code_toolbar.run_requested.connect(self.run_code_requested.emit)
        self._code_toolbar.reset_requested.connect(self.reset_code_requested.emit)
        self._markdown_toolbar.preview_requested.connect(self.preview_markdown_requested.emit)
        
        # Add toolbars to stack
        self._empty_index = self._stack.addWidget(self._empty_toolbar)
        self._code_index = self._stack.addWidget(self._code_toolbar)
        self._markdown_index = self._stack.addWidget(self._markdown_toolbar)
        
        # Start with empty toolbar
        self._stack.setCurrentIndex(self._empty_index)
    
    def show_empty_toolbar(self) -> None:
        """Show the empty toolbar (no cell selected)."""
        self._stack.setCurrentIndex(self._empty_index)
    
    def show_code_toolbar(self) -> None:
        """Show the code cell toolbar."""
        self._stack.setCurrentIndex(self._code_index)
    
    def show_markdown_toolbar(self) -> None:
        """Show the markdown cell toolbar."""
        self._stack.setCurrentIndex(self._markdown_index)
    
    def update_for_cell_type(self, cell_type: str | None) -> None:
        """Update toolbar based on selected cell type.
        
        Args:
            cell_type: Type of selected cell ("code", "markdown", or None)
        """
        if cell_type is None:
            self.show_empty_toolbar()
        elif cell_type == "code":
            self.show_code_toolbar()
        elif cell_type == "markdown":
            self.show_markdown_toolbar()
        else:
            self.show_empty_toolbar()
    
    def _apply_font(self, family: str, size: int) -> None:
        """Apply font to all toolbars and their children recursively.
        
        Args:
            family: Font family name
            size: Font size in points
        """
        font = QFont(family, size)
        
        # Apply to each toolbar
        for toolbar in [self._empty_toolbar, self._code_toolbar, self._markdown_toolbar]:
            toolbar.setFont(font)
            # Recursively apply to all child widgets (buttons, labels)
            for child in toolbar.findChildren(QWidget):
                child.setFont(font)
    
    def _on_ui_font_changed(self, family: str, size: int) -> None:
        """Handle UI font changes from FontService.
        
        Args:
            family: Font family name
            size: Font size in points
        """
        self._apply_font(family, size)
