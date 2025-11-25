"""Reusable widgets for sidebar layouts."""

from __future__ import annotations

from PySide6.QtWidgets import QWidget, QHBoxLayout, QSizePolicy


class SidebarActionRow(QWidget):
    """Simple horizontal row to host sidebar action buttons."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        # Allow row to expand horizontally but remain compact vertically
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self._layout = layout

    def add_widget(self, widget: QWidget) -> None:
        """Add a widget to the action row."""
        self._layout.addWidget(widget)

    def add_spacer(self) -> None:
        """Add a stretch spacer to push following widgets to the edge."""
        self._layout.addStretch(1)
