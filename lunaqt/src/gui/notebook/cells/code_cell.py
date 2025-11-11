"""CodeCell widget for executable code."""

from __future__ import annotations
from PySide6.QtWidgets import QWidget, QTextEdit, QLabel, QHBoxLayout
from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QFont

from .base_cell import BaseCell


class _CodeEditor(QTextEdit):
    """Text editor that notifies about focus changes."""

    focus_changed = Signal(bool)

    def focusInEvent(self, event):  # type: ignore[override]
        super().focusInEvent(event)
        self.focus_changed.emit(True)

    def focusOutEvent(self, event):  # type: ignore[override]
        super().focusOutEvent(event)
        self.focus_changed.emit(False)


class CodeCell(BaseCell):
    """Code cell with editor and execution support (future).
    
    Features:
    - Code editor with monospace font
    - Execution count indicator
    - Output area (future)
    """
    
    def __init__(
        self,
        cell_id: str,
        content: str = "",
        execution_count: int | None = None,
        parent: QWidget | None = None
    ) -> None:
        """Initialize code cell.
        
        Args:
            cell_id: Unique cell identifier.
            content: Initial code content.
            execution_count: Execution count (None if not run).
            parent: Parent widget.
        """
        super().__init__(cell_id, "code", parent)
        
        self._execution_count = execution_count
        self._setup_ui(content)
    
    def _setup_ui(self, content: str) -> None:
        """Set up the UI components.
        
        Args:
            content: Initial content.
        """
        # Header with execution count
        header_layout = QHBoxLayout()
        
        self._count_label = QLabel(self._format_count())
        self._count_label.setStyleSheet("color: palette(mid); font-size: 10pt;")
        header_layout.addWidget(self._count_label)
        header_layout.addStretch()
        
        self._content_layout.addLayout(header_layout)
        
        # Code editor
        self._editor = _CodeEditor()
        self._editor.setPlainText(content)
        self._editor.setAcceptRichText(False)
        self._editor.setTabStopDistance(40)  # 4 spaces
        
        # Set monospace font
        font = QFont("Consolas", 10)
        font.setStyleHint(QFont.StyleHint.Monospace)
        self._editor.setFont(font)
        
        # Connect text changes
        self._editor.textChanged.connect(self._on_text_changed)
        self._editor.focus_changed.connect(self._on_editor_focus_changed)
        
        self._content_layout.addWidget(self._editor)

    def focus_editor(self) -> None:
        try:
            self._editor.setFocus(Qt.FocusReason.OtherFocusReason)
        except Exception:
            super().focus_editor()
    
    def _format_count(self) -> str:
        """Format execution count for display.
        
        Returns:
            Formatted count string.
        """
        if self._execution_count is None:
            return "In [ ]:"
        return f"In [{self._execution_count}]:"
    
    def _on_text_changed(self) -> None:
        """Handle text changes in editor."""
        content = self._editor.toPlainText()
        self._emit_content_changed(content)
    
    def get_content(self) -> str:
        """Get cell content.
        
        Returns:
            Code content as string.
        """
        return self._editor.toPlainText()
    
    def set_content(self, content: str) -> None:
        """Set cell content.
        
        Args:
            content: New code content.
        """
        self._editor.setPlainText(content)
    
    def set_execution_count(self, count: int | None) -> None:
        """Set execution count.
        
        Args:
            count: Execution count or None.
        """
        self._execution_count = count
        self._count_label.setText(self._format_count())

    def clear_editor_focus(self) -> None:
        self._editor.clearFocus()
        super().clear_editor_focus()

    def _on_editor_focus_changed(self, has_focus: bool) -> None:
        if has_focus:
            self.set_selected(True)
