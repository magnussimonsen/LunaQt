"""MarkdownCell widget with edit/preview modes."""

from __future__ import annotations
from PySide6.QtWidgets import QWidget, QTextEdit, QTextBrowser, QPushButton, QHBoxLayout
from PySide6.QtCore import Qt, Signal

from .base_cell import BaseCell


class _MarkdownEditor(QTextEdit):
    focus_changed = Signal(bool)

    def focusInEvent(self, event):  # type: ignore[override]
        super().focusInEvent(event)
        self.focus_changed.emit(True)

    def focusOutEvent(self, event):  # type: ignore[override]
        super().focusOutEvent(event)
        self.focus_changed.emit(False)


class _MarkdownPreview(QTextBrowser):
    focus_changed = Signal(bool)

    def focusInEvent(self, event):  # type: ignore[override]
        super().focusInEvent(event)
        self.focus_changed.emit(True)

    def focusOutEvent(self, event):  # type: ignore[override]
        super().focusOutEvent(event)
        self.focus_changed.emit(False)


class MarkdownCell(BaseCell):
    """Markdown cell with edit and preview modes.
    
    Features:
    - Plain text editor for markdown
    - Preview mode with rendered HTML (basic for now)
    - Toggle button to switch modes
    """
    
    def __init__(
        self,
        cell_id: str,
        content: str = "",
        parent: QWidget | None = None
    ) -> None:
        """Initialize markdown cell.
        
        Args:
            cell_id: Unique cell identifier.
            content: Initial markdown content.
            parent: Parent widget.
        """
        super().__init__(cell_id, "markdown", parent)
        
        self._content = content
        self._is_preview_mode = False
        self._setup_ui()
    
    def _setup_ui(self) -> None:
        """Set up the UI components."""
        # Header with mode toggle
        header_layout = QHBoxLayout()
        
        self._toggle_button = QPushButton("Preview")
        self._toggle_button.clicked.connect(self._toggle_mode)
        header_layout.addWidget(self._toggle_button)
        header_layout.addStretch()
        
        self._content_layout.addLayout(header_layout)
        
        # Editor (edit mode)
        self._editor = _MarkdownEditor()
        self._editor.setPlainText(self._content)
        self._editor.setAcceptRichText(False)
        self._editor.textChanged.connect(self._on_text_changed)
        self._editor.focus_changed.connect(self._on_focus_changed)
        self._content_layout.addWidget(self._editor)
        
        # Preview (preview mode)
        self._preview = _MarkdownPreview()
        self._preview.setOpenExternalLinks(True)
        self._preview.setHtml(self._render_markdown(self._content))
        self._preview.hide()
        self._preview.focus_changed.connect(self._on_focus_changed)
        self._content_layout.addWidget(self._preview)

    def focus_editor(self) -> None:
        try:
            if not self._is_preview_mode:
                self._editor.setFocus(Qt.FocusReason.OtherFocusReason)
            else:
                self._preview.setFocus(Qt.FocusReason.OtherFocusReason)
        except Exception:
            super().focus_editor()
    
    def _toggle_mode(self) -> None:
        """Toggle between edit and preview modes."""
        self._is_preview_mode = not self._is_preview_mode
        
        if self._is_preview_mode:
            # Switch to preview
            self._content = self._editor.toPlainText()
            self._preview.setHtml(self._render_markdown(self._content))
            self._editor.hide()
            self._preview.show()
            self._toggle_button.setText("Edit")
        else:
            # Switch to edit
            self._editor.setPlainText(self._content)
            self._preview.hide()
            self._editor.show()
            self._toggle_button.setText("Preview")
            # Ensure focus moves back to the editor when returning to edit mode
            self._editor.setFocus(Qt.FocusReason.OtherFocusReason)
    
    def _render_markdown(self, text: str) -> str:
        """Render markdown to HTML (basic implementation).
        
        Args:
            text: Markdown text.
            
        Returns:
            HTML string.
        """
        # For MVP: basic HTML wrapping
        # Future: use markdown library for proper rendering
        html = text.replace("\n", "<br>")
        html = f"<div style='padding: 8px;'>{html}</div>"
        return html
    
    def _on_text_changed(self) -> None:
        """Handle text changes in editor."""
        if not self._is_preview_mode:
            content = self._editor.toPlainText()
            self._content = content
            self._emit_content_changed(content)
    
    def get_content(self) -> str:
        """Get cell content.
        
        Returns:
            Markdown content as string.
        """
        if self._is_preview_mode:
            return self._content
        return self._editor.toPlainText()
    
    def set_content(self, content: str) -> None:
        """Set cell content.
        
        Args:
            content: New markdown content.
        """
        self._content = content
        if not self._is_preview_mode:
            self._editor.setPlainText(content)
        else:
            self._preview.setHtml(self._render_markdown(content))

    def clear_editor_focus(self) -> None:
        self._editor.clearFocus()
        self._preview.clearFocus()
        super().clear_editor_focus()

    def _on_focus_changed(self, has_focus: bool) -> None:
        if has_focus:
            self.set_selected(True)
