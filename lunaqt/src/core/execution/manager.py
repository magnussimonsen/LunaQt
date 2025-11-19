"""Notebook execution manager orchestrating worker threads."""

from __future__ import annotations

from typing import Callable, Dict

from PySide6.QtCore import QObject, Signal

from .messages import ExecutionRequest, ExecutionResult
from .worker import ExecutionWorker


class NotebookExecutionManager(QObject):
    """High-level API for running code cells per notebook."""

    cell_started = Signal(object)   # ExecutionRequest
    cell_finished = Signal(object)  # ExecutionResult
    cell_failed = Signal(object)    # ExecutionResult

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self._workers: Dict[str, ExecutionWorker] = {}
        self._plot_style: dict[str, str] | None = None

    def set_plot_style(self, style: dict[str, str] | None) -> None:
        self._plot_style = dict(style) if style else None

    def run_cell(self, notebook_id: str, cell_id: str, code: str, execution_count: int | None = None) -> None:
        worker = self._get_or_create_worker(notebook_id)
        request = ExecutionRequest(
            notebook_id=notebook_id,
            cell_id=cell_id,
            code=code,
            execution_count=execution_count,
            plot_style=dict(self._plot_style) if self._plot_style else None,
        )
        worker.enqueue(request)

    def shutdown(self) -> None:
        for worker in self._workers.values():
            worker.shutdown()
        self._workers.clear()

    # Internal helpers --------------------------------------------------
    def _get_or_create_worker(self, notebook_id: str) -> ExecutionWorker:
        worker = self._workers.get(notebook_id)
        if worker is None:
            worker = ExecutionWorker(notebook_id)
            self._wire_worker_signals(worker)
            worker.start()
            self._workers[notebook_id] = worker
        return worker

    def _wire_worker_signals(self, worker: ExecutionWorker) -> None:
        worker.request_started.connect(self.cell_started.emit)
        worker.request_finished.connect(self.cell_finished.emit)
        worker.request_failed.connect(self.cell_failed.emit)
