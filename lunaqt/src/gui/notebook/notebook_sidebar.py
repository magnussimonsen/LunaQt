"""Sidebar widget for managing notebooks list and actions."""

from __future__ import annotations

from PySide6.QtCore import Qt, Signal
from PySide6.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QListWidget,
    QListWidgetItem,
    QAbstractItemView,
    QPushButton,
)

from ..sidebar.widgets import SidebarActionRow


class NotebookSidebarWidget(QWidget):
    """Notebook sidebar with add button and rename-capable list."""

    add_notebook_clicked = Signal()
    notebook_selected = Signal(str)
    rename_notebook_requested = Signal(str, str)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._is_updating = False
        self._setup_ui()

    def _setup_ui(self) -> None:
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(12)

        action_row = SidebarActionRow(self)
        add_button = QPushButton("Add New Notebook")
        add_button.clicked.connect(self.add_notebook_clicked)
        action_row.add_widget(add_button)
        action_row.add_spacer()
        layout.addWidget(action_row)

        list_widget = QListWidget(self)
        list_widget.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        list_widget.setEditTriggers(
            QAbstractItemView.EditTrigger.DoubleClicked
            | QAbstractItemView.EditTrigger.EditKeyPressed
        )
        list_widget.itemSelectionChanged.connect(self._on_selection_changed)
        list_widget.itemChanged.connect(self._on_item_changed)
        layout.addWidget(list_widget)

        self._list = list_widget
        self._add_button = add_button

    def set_notebooks(self, notebooks: list[dict[str, str]], active_notebook_id: str | None) -> None:
        """Populate the list with available notebooks."""
        self._is_updating = True
        self._list.clear()
        for notebook in notebooks:
            title = notebook.get("title") or "Untitled Notebook"
            item = QListWidgetItem(title)
            item.setData(Qt.ItemDataRole.UserRole, notebook.get("notebook_id"))
            item.setFlags(item.flags() | Qt.ItemIsEditable)
            self._list.addItem(item)
            if active_notebook_id and notebook.get("notebook_id") == active_notebook_id:
                self._list.setCurrentItem(item)
        self._is_updating = False

    def set_active_notebook(self, notebook_id: str) -> None:
        """Select the given notebook ID in the list if present."""
        if not notebook_id:
            return
        self._is_updating = True
        for idx in range(self._list.count()):
            item = self._list.item(idx)
            if item.data(Qt.ItemDataRole.UserRole) == notebook_id:
                self._list.setCurrentItem(item)
                break
        self._is_updating = False

    def _on_selection_changed(self) -> None:
        if self._is_updating:
            return
        current = self._list.currentItem()
        if current is None:
            return
        notebook_id = current.data(Qt.ItemDataRole.UserRole)
        if isinstance(notebook_id, str):
            self.notebook_selected.emit(notebook_id)

    def _on_item_changed(self, item: QListWidgetItem) -> None:
        if self._is_updating:
            return
        notebook_id = item.data(Qt.ItemDataRole.UserRole)
        if not isinstance(notebook_id, str):
            return
        new_title = item.text().strip()
        if not new_title:
            # Revert empty titles immediately
            self._is_updating = True
            item.setText("Untitled Notebook")
            self._is_updating = False
            new_title = "Untitled Notebook"
        self.rename_notebook_requested.emit(notebook_id, new_title)

    def focus_add_button(self) -> None:
        """Expose the add button for shortcuts/tests."""
        self._add_button.setFocus()
