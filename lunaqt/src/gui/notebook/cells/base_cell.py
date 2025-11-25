"""BaseCell template for all cell types.

Provides common functionality:
- Selection state management
- Visual selection indicator
- Signal emissions (selected, deleted, content_changed)
- Theme-aware styling
"""

from __future__ import annotations
from PySide6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QFrame, QLabel
from PySide6.QtCore import Signal, Qt
from PySide6.QtGui import QMouseEvent


class BaseCell(QFrame):
    """Base template for all cell types.
    
    Signals:
        selected: Emitted when cell is selected (cell_id, cell_type).
        deleted: Emitted when cell requests deletion (cell_id).
        content_changed: Emitted when content changes (cell_id, new_content).
    """
    
    selected = Signal(str, str)  # cell_id, cell_type
    deleted = Signal(str)  # cell_id
    content_changed = Signal(str, str)  # cell_id, new_content
    size_hint_changed = Signal(str)  # cell_id
    gutter_clicked = Signal(str)  # cell_id
    run_requested = Signal(str)  # cell_id
    
    def __init__(
        self,
        cell_id: str,
        cell_type: str,
        parent: QWidget | None = None
    ) -> None:
        """Initialize base cell.
        
        Args:
            cell_id: Unique cell identifier.
            cell_type: Cell type ('code', 'markdown', 'raw').
            parent: Parent widget.
        """
        super().__init__(parent)
        self._cell_id = cell_id
        self._cell_type = cell_type
        self._is_selected = False
        
        # Frame styling
        self.setFrameShape(QFrame.Shape.Box)
        self.setLineWidth(1)
        self._update_selection_style()

        # Main layout: left gutter + right content
        main_layout = QHBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # Gutter widget
        self._gutter = _Gutter()
        self._gutter.setFixedWidth(40)
        self._gutter.clicked.connect(lambda: self.gutter_clicked.emit(self._cell_id))
        main_layout.addWidget(self._gutter)

        # Content container with padding
        self._content_container = QWidget()
        self._content_layout = QVBoxLayout(self._content_container)
        self._content_layout.setContentsMargins(8, 8, 8, 8)
        self._content_layout.setSpacing(4)
        main_layout.addWidget(self._content_container, 1)
    
    @property
    def cell_id(self) -> str:
        """Get cell ID."""
        return self._cell_id
    
    @property
    def cell_type(self) -> str:
        """Get cell type."""
        return self._cell_type
    
    @property
    def is_selected(self) -> bool:
        """Get selection state."""
        return self._is_selected
    
    def set_selected(self, selected: bool) -> None:
        """Set selection state.
        
        Args:
            selected: Whether cell is selected.
        """
        if self._is_selected != selected:
            self._is_selected = selected
            self._update_selection_style()
            
            if selected:
                # Ensure the primary editor has focus when the cell is selected
                self.focus_editor()
                self.selected.emit(self._cell_id, self._cell_type)
            else:
                # Lose focus when the cell is deselected so the cursor disappears
                self.clear_editor_focus()
    
    def _update_selection_style(self) -> None:
        """Update visual style based on selection state."""
        # Use property selector to trigger QSS styling
        self.setProperty("selected", self._is_selected)
        # Force style refresh
        self.style().polish(self)
    
    def mousePressEvent(self, event: QMouseEvent) -> None:
        """Handle mouse press to select cell.
        
        Args:
            event: Mouse event.
        """
        if event.button() == Qt.MouseButton.LeftButton:
            self.set_selected(True)
        super().mousePressEvent(event)
    
    def get_content(self) -> str:
        """Get cell content. Override in subclasses.
        
        Returns:
            Cell content as string.
        """
        return ""
    
    def set_content(self, content: str) -> None:
        """Set cell content. Override in subclasses.
        
        Args:
            content: New content string.
        """
        pass
    
    def _emit_content_changed(self, content: str) -> None:
        """Emit content changed signal.
        
        Args:
            content: New content.
        """
        self.content_changed.emit(self._cell_id, content)

    def _notify_size_hint_changed(self) -> None:
        """Notify listeners that this cell's preferred size changed."""
        self.size_hint_changed.emit(self._cell_id)

    # --- Gutter API ---
    def set_index(self, index: int) -> None:
        """Set the visual index in the gutter (1-based)."""
        self._gutter.set_index(index)

    # --- Focus API ---
    def focus_editor(self) -> None:
        """Give focus to this cell's primary editor widget if any.

        Subclasses should override to set focus on their editor.
        """
        self.setFocus(Qt.FocusReason.OtherFocusReason)

    def clear_editor_focus(self) -> None:
        """Remove focus from this cell's primary editor widget."""
        self.clearFocus()


class _Gutter(QWidget):
    """Left-side gutter showing the cell index and capturing clicks to deselect."""

    clicked = Signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("CellGutter")  # Set object name for QSS targeting
        self._label = QLabel("")
        self._label.setAlignment(Qt.AlignCenter)
        lay = QVBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        lay.setSpacing(0)
        lay.addWidget(self._label)
        # Styling now handled by notebook_qss.py - don't override here

    def set_index(self, index: int) -> None:
        self._label.setText(str(index))

    def mousePressEvent(self, event: QMouseEvent) -> None:
        if event.button() == Qt.LeftButton:
            self.clicked.emit()
            event.accept()
            return
        super().mousePressEvent(event)
