"""CodeCell widget for executable code."""

from __future__ import annotations

from typing import TYPE_CHECKING

from PySide6.QtWidgets import (
    QWidget,
    QSizePolicy,
    QLabel,
    QPlainTextEdit,
    QVBoxLayout,
    QHBoxLayout,
    QSlider,
)
from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QFont, QPixmap

from .base_cell import BaseCell
from .python_editor import PythonCodeEditor
from ....core.font_service import get_font_service

if TYPE_CHECKING:  # pragma: no cover - import for typing only
    from ....core.execution.messages import ExecutionResult


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
        self._pending_execution_count: int | None = None
        self._plot_scale = 100
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
        # Execution count label mirrors classic notebook style
        self._count_label = QLabel(self._format_count())
        self._count_label.setObjectName("ExecutionCountLabel")
        self._count_label.setAlignment(Qt.AlignLeft)
        self._count_label.setProperty("monospace", True)
        self._content_layout.addWidget(self._count_label)

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
        self._editor.execute_requested.connect(self._on_execute_shortcut)
        
        self._content_layout.addWidget(self._editor)

        # Read-only output view shown after executions
        self._output_view = QPlainTextEdit()
        self._output_view.setReadOnly(True)
        self._output_view.setObjectName("CodeCellOutput")
        self._output_view.setVisible(False)
        self._output_view.setMaximumHeight(200)
        self._output_view.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        self._content_layout.addWidget(self._output_view)

        self._plot_controls = QWidget()
        controls_layout = QHBoxLayout(self._plot_controls)
        controls_layout.setContentsMargins(0, 0, 0, 0)
        controls_layout.setSpacing(6)
        self._plot_controls.setVisible(False)
        self._plot_scale_label = QLabel("Plot size: 100%")
        self._plot_scale_label.setObjectName("PlotScaleLabel")
        self._plot_scale_slider = QSlider(Qt.Orientation.Horizontal)
        self._plot_scale_slider.setRange(25, 100)
        self._plot_scale_slider.setSingleStep(5)
        self._plot_scale_slider.setValue(self._plot_scale)
        self._plot_scale_slider.setFixedWidth(120)
        self._plot_scale_slider.valueChanged.connect(self._on_plot_scale_changed)
        controls_layout.addWidget(self._plot_scale_slider)
        controls_layout.addWidget(self._plot_scale_label)
        self._content_layout.addWidget(self._plot_controls)

        self._plots_container = QWidget()
        self._plots_container.setObjectName("CodeCellPlots")
        self._plots_layout = QVBoxLayout(self._plots_container)
        self._plots_layout.setContentsMargins(0, 0, 0, 0)
        self._plots_layout.setSpacing(8)
        self._plots_container.setVisible(False)
        self._content_layout.addWidget(self._plots_container)
        self._plot_pixmaps: list[QPixmap] = []
        self._plot_labels: list[QLabel] = []
        
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

    def _update_count_label(self) -> None:
        self._count_label.setText(self._format_count())
    
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

    def get_execution_count(self) -> int | None:
        return self._execution_count
    
    def set_execution_count(self, count: int | None) -> None:
        """Set execution count.
        
        Args:
            count: Execution count or None.
        """
        if self._execution_count == count:
            return
        self._execution_count = count
        self._update_count_label()
        self._notify_size_hint_changed()

    def mark_execution_started(self, execution_count: int | None) -> None:
        self._pending_execution_count = execution_count
        self._count_label.setText("In [*]:")
        self.clear_output()

    def apply_execution_result(self, result: "ExecutionResult") -> None:
        self._pending_execution_count = None
        if result.execution_count is not None:
            self.set_execution_count(result.execution_count)
        self._display_output(result.stdout, result.stderr, result.error)
        self._display_plots(getattr(result, "figures", []))

    def mark_execution_failed(self, result: "ExecutionResult") -> None:
        self._pending_execution_count = None
        if result.execution_count is not None:
            self.set_execution_count(result.execution_count)
        fallback_error = result.error or "Execution failed"
        self._display_output(result.stdout, result.stderr, fallback_error)
        self._display_plots(getattr(result, "figures", []))

    def clear_output(self) -> None:
        self._output_view.clear()
        self._output_view.setVisible(False)
        self._clear_plots()
        self._notify_size_hint_changed()

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
        self._output_view.document().setDefaultFont(font)
        self._adjust_editor_height()

    def _on_execute_shortcut(self) -> None:
        self.run_requested.emit(self.cell_id)

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
            elif widget is self._output_view and not widget.isVisible():
                child_height = 0
            elif widget is self._plots_container and not widget.isVisible():
                child_height = 0
            elif widget is self._plot_controls and not widget.isVisible():
                child_height = 0
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

    def _display_output(self, stdout: str | None, stderr: str | None, error: str | None) -> None:
        chunks: list[str] = []
        if stdout:
            chunks.append(stdout.rstrip())
        if stderr:
            chunks.append(f"stderr:\n{stderr.rstrip()}")
        if error:
            chunks.append(f"error:\n{error.rstrip()}")
        if not chunks:
            self.clear_output()
            return
        self._output_view.setPlainText("\n\n".join(chunks))
        self._output_view.setVisible(True)
        self._notify_size_hint_changed()
        self._plots_container.setVisible(self._plots_layout.count() > 0)

    def _display_plots(self, figures: list[bytes]) -> None:
        self._clear_plots()
        if not figures:
            return
        for data in figures:
            pixmap = QPixmap()
            if not pixmap.loadFromData(data):
                continue
            label = QLabel()
            label.setAlignment(Qt.AlignLeft)
            label.setPixmap(pixmap)
            self._plots_layout.addWidget(label)
            self._plot_pixmaps.append(pixmap)
            self._plot_labels.append(label)
        self._plots_container.setVisible(True)
        self._plot_controls.setVisible(True)
        self._plot_scale_slider.blockSignals(True)
        self._plot_scale_slider.setValue(self._plot_scale)
        self._plot_scale_slider.blockSignals(False)
        self._plot_scale_label.setText(f"Plot size: {self._plot_scale}%")
        self._apply_plot_scale()
        self._notify_size_hint_changed()

    def _clear_plots(self) -> None:
        for label in self._plot_labels:
            label.deleteLater()
        self._plot_labels.clear()
        self._plot_pixmaps.clear()
        self._plots_container.setVisible(False)
        self._plot_controls.setVisible(False)

    def _on_plot_scale_changed(self, value: int) -> None:
        self._plot_scale = max(25, min(100, value))
        self._plot_scale_label.setText(f"Plot size: {self._plot_scale}%")
        self._apply_plot_scale()

    def _apply_plot_scale(self) -> None:
        if not self._plot_pixmaps or not self._plot_labels:
            return
        factor = self._plot_scale / 100.0
        for pixmap, label in zip(self._plot_pixmaps, self._plot_labels):
            if pixmap.isNull():
                continue
            target_width = max(1, int(pixmap.width() * factor))
            target_height = max(1, int(pixmap.height() * factor))
            scaled = pixmap.scaled(
                target_width,
                target_height,
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation,
            )
            label.setPixmap(scaled)
            label.setMinimumSize(scaled.size())
            label.setMaximumSize(scaled.size())
            label.adjustSize()
        self._notify_size_hint_changed()
