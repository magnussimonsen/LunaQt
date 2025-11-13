"""MarkdownCell widget with edit/preview modes."""

from __future__ import annotations
from PySide6.QtWidgets import QWidget, QTextEdit, QTextBrowser
from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QFont

from .base_cell import BaseCell
from ....core.font_service import get_font_service


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
        
        # Subscribe to font service
        self._font_service = get_font_service()
        self._font_service.textFontChanged.connect(self._on_text_font_changed)
        # Apply current font from service
        family, size = self._font_service.get_text_font()
        if family and size:
            self._apply_font(family, size)
    
    def _setup_ui(self) -> None:
        """Set up the UI components."""
        # Editor (edit mode)
        self._editor = _MarkdownEditor()
        self._editor.setPlainText(self._content)
        self._editor.setAcceptRichText(False)
        
        # Set size policy and minimum height
        from PySide6.QtWidgets import QSizePolicy
        self._editor.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        self._editor.setMinimumHeight(50)
        self._editor.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self._editor.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        
        self._editor.textChanged.connect(self._on_text_changed)
        self._editor.textChanged.connect(self._adjust_editor_height)
        self._editor.focus_changed.connect(self._on_focus_changed)
        self._content_layout.addWidget(self._editor)
        
        # Preview (preview mode)
        self._preview = _MarkdownPreview()
        self._preview.setOpenExternalLinks(True)
        self._preview.setHtml(self._render_markdown(self._content))
        self._preview.hide()
        
        # Set size policy and minimum height for preview
        self._preview.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        self._preview.setMinimumHeight(50)
        self._preview.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self._preview.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        
        self._preview.focus_changed.connect(self._on_focus_changed)
        self._content_layout.addWidget(self._preview)
        
        # Initial height adjustment
        self._adjust_editor_height()

    def focus_editor(self) -> None:
        try:
            if not self._is_preview_mode:
                self._editor.setFocus(Qt.FocusReason.OtherFocusReason)
            else:
                self._preview.setFocus(Qt.FocusReason.OtherFocusReason)
        except Exception:
            super().focus_editor()
    
    def _adjust_editor_height(self) -> None:
        """Adjust editor height to fit content."""
        if not self._is_preview_mode:
            doc_height = self._editor.document().size().height()
            new_height = int(doc_height + 10)
            new_height = max(new_height, 50)
            self._editor.setFixedHeight(new_height)
    
    def _adjust_preview_height(self) -> None:
        """Adjust preview height to fit content."""
        if self._is_preview_mode:
            doc_height = self._preview.document().size().height()
            new_height = int(doc_height + 10)
            new_height = max(new_height, 50)
            self._preview.setFixedHeight(new_height)
    
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
            self._adjust_preview_height()
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

    def _on_text_font_changed(self, family: str, size: int) -> None:
        """Handle text font changes from font service.
        
        Args:
            family: Font family name.
            size: Font size in points.
        """
        self._apply_font(family, size)
    
    def _apply_font(self, family: str, size: int) -> None:
        """Apply font to editor and preview.
        
        Args:
            family: Font family name.
            size: Font size in points.
        """
        font = QFont(family, size)
        self._editor.setFont(font)
        self._preview.setFont(font)
