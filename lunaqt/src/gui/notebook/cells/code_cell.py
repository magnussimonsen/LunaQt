"""CodeCell widget for executable code."""

from __future__ import annotations
from PySide6.QtWidgets import QWidget, QSizePolicy
from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QFont

from .base_cell import BaseCell
from .python_editor import PythonCodeEditor
from ....core.font_service import get_font_service


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
        
        # Subscribe to font service
        self._font_service = get_font_service()
        self._font_service.codeFontChanged.connect(self._on_code_font_changed)
        # Apply current font from service
        family, size = self._font_service.get_code_font()
        if family and size:
            self._apply_font(family, size)
    
    def _setup_ui(self, content: str) -> None:
        """Set up the UI components.
        
        Args:
            content: Initial content.
        """
        # Code editor with syntax highlighting
        self._editor = PythonCodeEditor()
        self._editor.setPlainText(content)
        
        # Set size policy to expand vertically, fit horizontally
        self._editor.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        
        # Set minimum height (approximately 2 lines)
        self._editor.setMinimumHeight(50)
        
        # Allow the widget to grow instead of showing a vertical scrollbar
        self._editor.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self._editor.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        
        # Font will be set by font service in __init__
        
        # Connect text changes for content/height updates
        self._editor.textChanged.connect(self._on_text_changed)
        self._editor.focus_changed.connect(self._on_editor_focus_changed)
        
        self._content_layout.addWidget(self._editor)
        
        # Initial height adjustment
        self._adjust_editor_height()

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
    
    def _adjust_editor_height(self) -> None:
        """Adjust editor height to fit content."""
        # Get document height
        doc = self._editor.document()
        block_count = max(1, doc.blockCount())
        line_height = self._editor.fontMetrics().lineSpacing()
        doc_height = line_height * block_count

        margins = self._editor.contentsMargins()
        frame = self._editor.frameWidth() * 2
        padding = margins.top() + margins.bottom() + frame + int(doc.documentMargin() * 2) + 4
        new_height = int(doc_height + padding)
        new_height = max(new_height, 50)
        self._editor.setFixedHeight(new_height)
        self._editor.updateGeometry()
        self.updateGeometry()
        self._notify_size_hint_changed()
    
    def _on_text_changed(self) -> None:
        """Handle text changes in editor."""
        self._adjust_editor_height()
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

    def _on_code_font_changed(self, family: str, size: int) -> None:
        """Handle code font changes from font service.
        
        Args:
            family: Font family name.
            size: Font size in points.
        """
        self._apply_font(family, size)
    
    def _apply_font(self, family: str, size: int) -> None:
        """Apply font to the code editor.
        
        Args:
            family: Font family name.
            size: Font size in points.
        """
        font = QFont(family, size)
        font.setStyleHint(QFont.StyleHint.Monospace)
        if hasattr(self._editor, "set_font"):
            self._editor.set_font(font)
        else:
            self._editor.setFont(font)
        self._adjust_editor_height()

    # Qt sizing ----------------------------------------------------------
    def sizeHint(self) -> QSize:  # type: ignore[override]
        base = super().sizeHint()
        base.setHeight(self._dynamic_height())
        return base

    def minimumSizeHint(self) -> QSize:  # type: ignore[override]
        base = super().minimumSizeHint()
        base.setHeight(self._dynamic_height())
        return base

    def _dynamic_height(self) -> int:
        height = 0
        layout = self._content_layout
        count = layout.count()
        spacing = max(0, layout.spacing())
        for i in range(count):
            item = layout.itemAt(i)
            if not item:
                continue
            widget = item.widget()
            if widget is self._editor:
                child_height = self._editor.height()
            elif widget is not None:
                child_height = widget.sizeHint().height()
            else:
                child_height = item.sizeHint().height()
            height += max(0, child_height)
            if i < count - 1:
                height += spacing

        margins = layout.contentsMargins()
        height += margins.top() + margins.bottom()
        height += self.frameWidth() * 2
        return height
