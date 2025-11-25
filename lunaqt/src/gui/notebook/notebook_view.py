"""NotebookView container for notebook cells (QListWidget-based).

Implements MVP behavior per plan:
- Single selection enforcement
- Registry mapping between cell_id, items and widgets
- Insert Above/Below/End (UI helpers + optional persistence via managers)
- Delete selected with sensible fallback selection
- Move Up/Down with boundary guards, preserving selection
- Placeholder for empty notebook with quick-add buttons
- Signals for selection/state changes
"""

from __future__ import annotations
from PySide6.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QListWidget,
    QListWidgetItem,
    QLabel,
)
from PySide6.QtCore import Signal, Qt
from PySide6.QtGui import QWheelEvent
import shiboken6
import logging
from typing import Any

from .cells.base_cell import BaseCell
from .cells.code_cell import CodeCell
from .cells.markdown_cell import MarkdownCell
from ...core.execution.manager import NotebookExecutionManager
from ...core.execution.messages import ExecutionRequest, ExecutionResult
from ...constants.matplotlib_styles import (
    DEFAULT_MPL_STYLE_NAME,
    get_matplotlib_style,
)

# Logger for NotebookView
logger = logging.getLogger(__name__)


# Smooth scrolling list widget -----------------------------------------------
class _SmoothListWidget(QListWidget):
    """QListWidget variant that scrolls in pixel increments via mouse wheel."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._wheel_pixels = 30  # pixels per wheel step

    def wheelEvent(self, event: QWheelEvent) -> None:  # type: ignore[override]
        delta = event.angleDelta().y()
        if delta == 0 and event.pixelDelta().y() != 0:
            delta = event.pixelDelta().y()
        if delta == 0:
            super().wheelEvent(event)
            return
        step = int(self._wheel_pixels * (delta / 120))
        new_value = self.verticalScrollBar().value() - step
        self.verticalScrollBar().setValue(new_value)
        event.accept()


# Optional managers (set via set_managers). Lazy imports to avoid cycles at import time.
try:  # pragma: no cover - optional at runtime
    from ...core.notebook_manager import NotebookManager
    from ...core.cell_manager import CellManager
except Exception:  # pragma: no cover - tools may not resolve during static analysis
    NotebookManager = object  # type: ignore
    CellManager = object  # type: ignore


class NotebookView(QWidget):
    """Container widget for displaying notebook cells.

    Signals:
        selection_changed: Emitted when selection changes (cell_id, cell_type, index)
        state_changed: Emitted when state toggles enablement
        cell_selected: Legacy alias for selection (cell_id, cell_type)
    """

    # New signals
    selection_changed = Signal(str, str, int)
    state_changed = Signal(bool, bool, bool, bool, int, bool)
    # Back-compat alias; emit alongside selection_changed
    cell_selected = Signal(str, str)  # cell_id, cell_type
    
    def __init__(self, parent: QWidget | None = None) -> None:
        """Initialize notebook view.
        
        Args:
            parent: Parent widget.
        """
        super().__init__(parent)

        # Managers and active notebook context (optional)
        self._notebook_manager = None  # type: Any | None
        self._cell_manager = None      # type: Any | None
        self._active_notebook_id = None  # type: str | None

        # Registries
        self._id_to_item = {}    # type: dict[str, QListWidgetItem]
        self._id_to_widget = {}  # type: dict[str, BaseCell]
        self._running_cells: set[str] = set()

        # Execution management
        self._execution_manager = NotebookExecutionManager(self)
        self._execution_manager.cell_started.connect(self._on_execution_started)
        self._execution_manager.cell_finished.connect(self._on_execution_finished)
        self._execution_manager.cell_failed.connect(self._on_execution_failed)
        self.destroyed.connect(self._on_view_destroyed)
        self._plot_theme = DEFAULT_MPL_STYLE_NAME
        self._current_plot_style = get_matplotlib_style(self._plot_theme)
        self._execution_manager.set_plot_style(self._current_plot_style)

        self._setup_ui()
    
    def _setup_ui(self) -> None:
        """Set up the UI components."""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        # Placeholder (visible when no cells)
        self._placeholder = self._create_placeholder()
        layout.addWidget(self._placeholder)

        # QListWidget for cell widgets
        self._cell_list = _SmoothListWidget()
        self._cell_list.setSelectionMode(QListWidget.SelectionMode.SingleSelection)
        self._cell_list.itemSelectionChanged.connect(self._on_qt_selection_changed)
        # Prevent blue selection background from painting over embedded widgets
        self._cell_list.setStyleSheet(
            "QListWidget::item { background: transparent; }\n"
            "QListWidget::item:selected { background: transparent; }"
        )
        
        # Enable smooth pixel-based scrolling
        self._cell_list.setVerticalScrollMode(QListWidget.ScrollMode.ScrollPerPixel)
        layout.addWidget(self._cell_list)

        self._update_placeholder_visibility()

    def _create_placeholder(self) -> QWidget:
        """Create a centered placeholder with guidance to use menu actions."""
        w = QWidget()
        v = QVBoxLayout(w)
        v.setContentsMargins(24, 24, 24, 24)
        v.setSpacing(12)
        w.setStyleSheet("color: palette(mid);")

        label = QLabel("No cells yet\nUse the Insert menu to add a cell")
        label.setAlignment(Qt.AlignHCenter)
        v.addWidget(label)

        return w

    def _update_placeholder_visibility(self) -> None:
        is_empty = self._cell_list.count() == 0
        self._placeholder.setVisible(is_empty)
        self._cell_list.setVisible(not is_empty)
    
    def _on_qt_selection_changed(self) -> None:
        """Handle list selection change and emit signals."""
        idx = self.get_selected_index()
        if idx is None:
            logger.debug("selection_changed: none selected")
            self._apply_cell_selection(None)
            self._emit_state_changed()
            return
        item = self._cell_list.item(idx)
        data = item.data(Qt.UserRole)
        cell_id = data if isinstance(data, str) else None
        if not cell_id:
            logger.debug("selection_changed: item without cell_id at index %s", idx)
            self._emit_state_changed()
            return
        widget = self._id_to_widget.get(cell_id)
        if not widget:
            logger.debug("selection_changed: no widget for cell_id %s", cell_id)
            self._emit_state_changed()
            return
        # Ensure BaseCell selection reflects QListWidget selection
        self._apply_cell_selection(cell_id)
        logger.debug("selection_changed: index=%s cell_id=%s cell_type=%s", idx, cell_id, getattr(widget, 'cell_type', '?'))
        self.selection_changed.emit(cell_id, widget.cell_type, idx)
        self.cell_selected.emit(cell_id, widget.cell_type)
        self._emit_state_changed()
    
    def add_cell_widget(self, cell_widget: BaseCell, cell_id: str, position: int | None = None) -> None:
        """Add a BaseCell widget into the list at position.

        If position is None or out of range, appends to the end.
        """
        item = QListWidgetItem()
        insert_at = self._cell_list.count() if position is None else max(0, min(position, self._cell_list.count()))
        logger.debug("add_cell_widget: cell_id=%s type=%s position=%s", cell_id, getattr(cell_widget, 'cell_type', '?'), insert_at)
        self._cell_list.insertItem(insert_at, item)
        # Explicitly set flags to enabled/selectable
        item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._cell_list.setItemWidget(item, cell_widget)
        # Ensure the row height matches the widget's preferred size
        try:
            item.setSizeHint(cell_widget.sizeHint())
            # Recompute layout so clicks map to correct rows
            self._cell_list.doItemsLayout()
        except Exception:
            pass

        # Registry updates
        self._id_to_item[cell_id] = item
        self._id_to_widget[cell_id] = cell_widget
        # Store cell_id on the item to avoid separate item->id dict
        item.setData(Qt.UserRole, cell_id)
        item.setData(Qt.UserRole + 1, cell_widget.cell_type)

        # Wire cell signals
        self._wire_cell_widget_signals(cell_widget)

        self._update_placeholder_visibility()
        self._emit_state_changed()
        self._refresh_indices()
    
    def clear_cells(self) -> None:
        """Clear all cells from the view and reset registries."""
        self._cell_list.clear()
        self._id_to_item.clear()
        self._id_to_widget.clear()
        self._update_placeholder_visibility()
        self._emit_state_changed()
        self._refresh_indices()

    def _wire_cell_widget_signals(self, cell_widget: BaseCell) -> None:
        """Connect signals from a cell widget to the view handlers.
        
        Args:
            cell_widget: The cell widget to wire up.
        """
        cell_widget.selected.connect(self._on_cell_widget_selected)
        cell_widget.content_changed.connect(self._on_cell_content_changed)
        cell_widget.size_hint_changed.connect(self._on_cell_size_hint_changed)
        cell_widget.gutter_clicked.connect(self._on_cell_gutter_clicked)
        cell_widget.run_requested.connect(self._on_cell_run_requested)

    # ----- Public helpers per plan -----
    def get_selected_index(self) -> int | None:
        items = self._cell_list.selectedItems()
        if not items:
            return None
        item = items[0]
        return self._cell_list.row(item)

    def get_selected_cell_id(self) -> str | None:
        idx = self.get_selected_index()
        if idx is None:
            return None
        item = self._cell_list.item(idx)
        data = item.data(Qt.UserRole)
        return data if isinstance(data, str) else None

    def select_index(self, i: int) -> None:
        if i < 0 or i >= self._cell_list.count():
            self._cell_list.clearSelection()
            self._emit_state_changed()
            return
        item = self._cell_list.item(i)
        self._cell_list.setCurrentItem(item)

    def select_cell(self, cell_id: str) -> None:
        item = self._id_to_item.get(cell_id)
        if not item:
            return
        self._cell_list.setCurrentItem(item)

    # ----- Persistence context -----
    def set_managers(self, notebook_manager: Any, cell_manager: Any) -> None:
        self._notebook_manager = notebook_manager
        self._cell_manager = cell_manager

    def set_active_notebook(self, notebook_id: str) -> None:
        self._active_notebook_id = notebook_id
        self.load_notebook(notebook_id)
        if notebook_id:
            # Ensure execution manager uses distinct worker per notebook
            logger.debug("NotebookView: active notebook set to %s", notebook_id)

    def set_plot_theme(self, theme: str) -> None:
        """Adjust matplotlib styling to match the current UI theme."""
        if not theme:
            theme = DEFAULT_MPL_STYLE_NAME
        self._plot_theme = theme
        self._update_plot_style(theme)

    def load_notebook(self, notebook_id: str) -> None:
        """Load cells from managers into the UI (if managers provided)."""
        self.clear_cells()
        if not (self._notebook_manager and self._cell_manager):
            self._update_placeholder_visibility()
            return
        order = self._notebook_manager.get_cell_order(notebook_id)
        for cid in order:
            cdata = self._cell_manager.get_cell(cid)
            if not cdata:
                continue
            widget = self._create_widget_from_data(cdata)
            if widget is None:
                continue
            self.add_cell_widget(widget, cid)
        # Select first if any
        if self._cell_list.count() > 0:
            self.select_index(0)
        else:
            self._update_placeholder_visibility()
            self._emit_state_changed()
        self._refresh_indices()

    # ----- Operations: insert/delete/move -----
    def insert_above(self, cell_type: str) -> None:
        idx = self.get_selected_index()
        if idx is None:
            self.insert_at_end(cell_type)
            return
        self._insert_at(cell_type, idx)

    def insert_below(self, cell_type: str) -> None:
        idx = self.get_selected_index()
        if idx is None:
            self.insert_at_end(cell_type)
            return
        self._insert_at(cell_type, idx + 1)

    def insert_at_end(self, cell_type: str) -> None:
        self._insert_at(cell_type, self._cell_list.count())

    def _insert_at(self, cell_type: str, position: int) -> None:
        # Create via manager if available, otherwise create a temporary UI-only cell
        cell_id: str
        content = ""
        if self._cell_manager and self._notebook_manager and self._active_notebook_id:
            cell_id = self._cell_manager.create_cell(cell_type=cell_type, content="")
            self._notebook_manager.add_cell(self._active_notebook_id, cell_id, position)
            cdata = self._cell_manager.get_cell(cell_id)
            if cdata:
                content = cdata.get("content", "")
        else:
            # UI-only fallback (non-persistent). Use a synthetic id.
            cell_id = f"ui-{self._cell_list.count()}-{cell_type}"
        widget = self._create_widget(cell_type, cell_id, content)
        self.add_cell_widget(widget, cell_id, position)
        self.select_cell(cell_id)

    def delete_selected(self) -> None:
        idx = self.get_selected_index()
        if idx is None:
            return
        item = self._cell_list.item(idx)
        data = item.data(Qt.UserRole)
        cell_id = data if isinstance(data, str) else None
        if not cell_id:
            return
        # Persist removal
        if self._notebook_manager and self._active_notebook_id:
            self._notebook_manager.remove_cell(self._active_notebook_id, cell_id)
        if self._cell_manager and cell_id and not cell_id.startswith("ui-"):
            # Only delete persisted cells
            try:
                self._cell_manager.delete_cell(cell_id)
            except Exception:
                pass
        # Remove UI
        self._remove_by_index(idx)
        # Fallback selection
        new_count = self._cell_list.count()
        if new_count == 0:
            self._cell_list.clearSelection()
            self._update_placeholder_visibility()
            self._emit_state_changed()
            return
        # Try same index, else previous
        new_index = idx if idx < new_count else new_count - 1
        self.select_index(new_index)

    def move_selected_up(self) -> None:
        self._move_selected(-1)

    def move_selected_down(self) -> None:
        self._move_selected(+1)

    def _move_selected(self, delta: int) -> None:
        idx = self.get_selected_index()
        if idx is None:
            logger.debug("move_selected: no selection, delta=%s", delta)
            return
        new_index = idx + delta
        if new_index < 0 or new_index >= self._cell_list.count():
            logger.debug("move_selected: out of bounds idx=%s new_index=%s count=%s", idx, new_index, self._cell_list.count())
            return
        
        # Get the cell_id being moved
        item = self._cell_list.item(idx)
        if item is None:
            logger.debug("move_selected: no QListWidgetItem at idx=%s", idx)
            return
        cell_id = item.data(Qt.UserRole) if isinstance(item.data(Qt.UserRole), str) else None
        
        if not cell_id:
            logger.debug("move_selected: no cell_id found")
            return
        
        logger.debug("move_selected start: idx=%s new_index=%s cell_id=%s", idx, new_index, cell_id)
        
        # Update persistence layer when available
        if self._notebook_manager and self._active_notebook_id:
            self._notebook_manager.move_cell(self._active_notebook_id, cell_id, new_index)
            logger.debug("persist move: cell_id=%s new_index=%s", cell_id, new_index)
        else:
            logger.debug("move_selected: no manager context, performing UI-only move")

        widget = self._cell_list.itemWidget(item)
        if not widget or not shiboken6.isValid(widget):
            widget = self._recreate_widget_for_cell(cell_id)

        taken_item = self._cell_list.takeItem(idx)
        if taken_item is None:
            logger.debug("move_selected: takeItem returned None, aborting move")
            return

        insert_index = max(0, min(new_index, self._cell_list.count()))

        # Reinsert the existing QListWidgetItem to avoid rebuilding the entire list
        self._cell_list.insertItem(insert_index, taken_item)
        if widget is not None:
            self._cell_list.setItemWidget(taken_item, widget)
            try:
                taken_item.setSizeHint(widget.sizeHint())
            except Exception:
                pass
        self._id_to_item[cell_id] = taken_item

        self.select_index(insert_index)
        self._refresh_indices()
        try:
            self._cell_list.doItemsLayout()
            self._cell_list.viewport().update()
        except Exception:
            pass
        logger.debug("move_selected done: cell_id=%s new_index=%s", cell_id, insert_index)

        self._emit_state_changed()

    # ----- Execution API -----
    def run_selected_cell(self) -> None:
        cell_id = self.get_selected_cell_id()
        if not cell_id:
            logger.debug("run_selected_cell: no selection")
            return
        self.run_cell(cell_id)

    def run_cell(self, cell_id: str) -> None:
        if not self._active_notebook_id:
            logger.warning("run_cell: no active notebook, cell_id=%s", cell_id)
            return
        widget = self._get_code_cell_widget(cell_id)
        if widget is None:
            logger.debug("run_cell: widget unavailable or not code cell cell_id=%s", cell_id)
            return

        code = widget.get_content() or ""
        prev_count = widget.get_execution_count() or 0
        next_count = prev_count + 1

        try:
            widget.mark_execution_started(next_count)
            self._running_cells.add(cell_id)
            self._execution_manager.run_cell(
                notebook_id=self._active_notebook_id,
                cell_id=cell_id,
                code=code,
                execution_count=next_count,
            )
        except Exception as exc:  # pragma: no cover - defensive guard
            logger.exception("run_cell: failed to dispatch execution cell_id=%s", cell_id)
            self._running_cells.discard(cell_id)
            widget.mark_execution_failed(
                ExecutionResult(
                    notebook_id=self._active_notebook_id,
                    cell_id=cell_id,
                    execution_count=next_count,
                    error=str(exc),
                )
            )

    # ----- Internals -----
    def _remove_by_index(self, idx: int) -> None:
        item = self._cell_list.item(idx)
        if not item:
            return
        # Delete associated widget explicitly to avoid dangling Python wrappers
        w = self._cell_list.itemWidget(item)
        if w is not None:
            w.deleteLater()
        # Now remove the item
        self._cell_list.takeItem(idx)
        data = item.data(Qt.UserRole)
        cell_id = data if isinstance(data, str) else None
        if cell_id:
            self._id_to_item.pop(cell_id, None)
            self._id_to_widget.pop(cell_id, None)
        del item
        self._update_placeholder_visibility()
        try:
            self._cell_list.doItemsLayout()
            self._cell_list.viewport().update()
        except Exception:
            pass
        self._refresh_indices()

    def _create_widget_from_data(self, data: dict) -> BaseCell | None:
        cell_type = data.get("cell_type")
        cell_id = data.get("cell_id")
        content = data.get("content", "")
        if not cell_type or not cell_id:
            return None
        return self._create_widget(cell_type, cell_id, content)

    def _recreate_widget_for_cell(self, cell_id: str) -> BaseCell | None:
        """Ensure there is a valid widget instance for the given cell id."""
        logger.debug("recreate_widget_for_cell: start cell_id=%s", cell_id)
        widget = self._id_to_widget.get(cell_id)
        if widget and shiboken6.isValid(widget):
            logger.debug("recreate_widget_for_cell: existing valid widget")
            return widget

        item = self._id_to_item.get(cell_id)
        stored_type = None
        if item:
            stored_type = item.data(Qt.UserRole + 1)
            if not isinstance(stored_type, str):
                stored_type = None

        cell_type = stored_type
        content = ""
        if self._cell_manager and cell_id and not cell_id.startswith("ui-"):
            try:
                cell_data = self._cell_manager.get_cell(cell_id)
            except Exception:
                cell_data = None
            if cell_data:
                cell_type = cell_data.get("cell_type", cell_type)
                content = cell_data.get("content", "")

        if cell_type is None and widget:
            cell_type = widget.cell_type
        if cell_type is None:
            cell_type = "code"

        logger.debug("recreate_widget_for_cell: creating cell_type=%s content_len=%s", cell_type, len(content))
        new_widget = self._create_widget(cell_type, cell_id, content)
        self._wire_cell_widget_signals(new_widget)
        self._id_to_widget[cell_id] = new_widget
        logger.debug("recreate_widget_for_cell: done")
        return new_widget

    def _get_code_cell_widget(self, cell_id: str) -> CodeCell | None:
        widget = self._id_to_widget.get(cell_id)
        if not isinstance(widget, CodeCell) or not shiboken6.isValid(widget):
            widget = self._recreate_widget_for_cell(cell_id)
        if isinstance(widget, CodeCell):
            return widget
        return None

    def _create_widget(self, cell_type: str, cell_id: str, content: str) -> BaseCell:
        if cell_type == "code":
            exec_count = None
            return CodeCell(cell_id=cell_id, content=content, execution_count=exec_count)
        elif cell_type == "markdown":
            return MarkdownCell(cell_id=cell_id, content=content)
        else:
            # Fallback to markdown for unknown types
            return MarkdownCell(cell_id=cell_id, content=content)

    def _on_cell_widget_selected(self, cell_id: str, cell_type: str) -> None:
        # Ensure list selection matches clicked cell
        item = self._id_to_item.get(cell_id)
        if item:
            self._cell_list.setCurrentItem(item)

    def _on_cell_content_changed(self, cell_id: str, new_content: str) -> None:
        # Persist content change if possible
        if self._cell_manager:
            try:
                self._cell_manager.update_cell(cell_id, content=new_content)
            except Exception:
                pass
        # Ensure the item's size matches new content to keep click mapping correct
        self._refresh_item_size_hint(cell_id)

    def _on_cell_run_requested(self, cell_id: str) -> None:
        self.run_cell(cell_id)

    def _on_cell_size_hint_changed(self, cell_id: str) -> None:
        self._refresh_item_size_hint(cell_id)

    def _refresh_item_size_hint(self, cell_id: str) -> None:
        try:
            item = self._id_to_item.get(cell_id)
            widget = self._id_to_widget.get(cell_id)
            if item is not None and isinstance(widget, BaseCell) and shiboken6.isValid(widget):
                item.setSizeHint(widget.sizeHint())
                self._cell_list.doItemsLayout()
                self._cell_list.viewport().update()
        except Exception:
            pass

    def _apply_cell_selection(self, selected_id: str | None) -> None:
        """Update BaseCell selection visuals.
        
        Args:
            selected_id: ID of cell to select, or None to deselect all.
        """
        # Update BaseCell selection visuals
        for cid, widget in list(self._id_to_widget.items()):
            try:
                # Check validity
                if not shiboken6.isValid(widget):
                    logger.debug("_apply_cell_selection: removing stale widget cell_id=%s", cid)
                    self._id_to_widget.pop(cid, None)
                    continue
                widget.set_selected(selected_id is not None and cid == selected_id)
            except RuntimeError:
                # C++ object deleted; remove stale mapping
                logger.debug("_apply_cell_selection: removing deleted widget cell_id=%s", cid)
                self._id_to_widget.pop(cid, None)
                continue

    def _refresh_indices(self) -> None:
        """Update the visible indices in each cell gutter to match list order.
        
        Updates each cell's gutter to display its 1-based position in the notebook.
        """
        count = self._cell_list.count()
        for i in range(count):
            item = self._cell_list.item(i)
            widget = self._cell_list.itemWidget(item)
            if isinstance(widget, BaseCell):
                widget.set_index(i + 1)

    def _on_cell_gutter_clicked(self, cell_id: str) -> None:
        """Deselect when the gutter is clicked."""
        self._cell_list.clearSelection()
        # Clear selection visuals
        for cid, widget in list(self._id_to_widget.items()):
            try:
                widget.set_selected(False)
            except RuntimeError:
                self._id_to_widget.pop(cid, None)
        self._emit_state_changed()

    def _emit_state_changed(self) -> None:
        count = self._cell_list.count()
        idx = self.get_selected_index()
        has_sel = idx is not None
        can_insert = True
        can_delete = has_sel and count > 0
        can_move_up = has_sel and idx is not None and idx > 0
        can_move_down = has_sel and idx is not None and idx < (count - 1)
        self.state_changed.emit(can_insert, can_delete, can_move_up, can_move_down, count, has_sel)

    def _update_plot_style(self, theme: str) -> None:
        style = get_matplotlib_style(theme)
        self._current_plot_style = style
        self._execution_manager.set_plot_style(style)

    # ----- Execution signal handlers ---------------------------------
    def _on_execution_started(self, payload: object) -> None:
        if not isinstance(payload, ExecutionRequest):
            return
        if payload.notebook_id != self._active_notebook_id:
            return
        widget = self._get_code_cell_widget(payload.cell_id)
        already_tracking = payload.cell_id in self._running_cells
        self._running_cells.add(payload.cell_id)
        if widget and not already_tracking:
            widget.mark_execution_started(payload.execution_count)

    def _on_execution_finished(self, payload: object) -> None:
        if not isinstance(payload, ExecutionResult):
            return
        if payload.notebook_id != self._active_notebook_id:
            return
        widget = self._get_code_cell_widget(payload.cell_id)
        if widget:
            widget.apply_execution_result(payload)
        self._running_cells.discard(payload.cell_id)

    def _on_execution_failed(self, payload: object) -> None:
        if not isinstance(payload, ExecutionResult):
            return
        if payload.notebook_id != self._active_notebook_id:
            return
        widget = self._get_code_cell_widget(payload.cell_id)
        if widget:
            widget.mark_execution_failed(payload)
        self._running_cells.discard(payload.cell_id)

    def _on_view_destroyed(self, *_args: object) -> None:
        try:
            self._execution_manager.shutdown()
        except Exception:  # pragma: no cover - best effort cleanup
            pass
