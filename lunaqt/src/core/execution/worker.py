"""Background worker responsible for running notebook cells."""

from __future__ import annotations

import contextlib
import io
import os
import queue
import traceback
from threading import Event
from typing import Optional

from PySide6.QtCore import QThread, Signal

from .messages import ExecutionRequest, ExecutionResult


class ExecutionWorker(QThread):
    """Dedicated worker thread tied to a specific notebook."""

    request_started = Signal(object)   # ExecutionRequest
    request_finished = Signal(object)  # ExecutionResult
    request_failed = Signal(object)    # ExecutionResult

    def __init__(self, notebook_id: str, parent=None) -> None:
        super().__init__(parent)
        self._notebook_id = notebook_id
        self._queue: queue.Queue[Optional[ExecutionRequest]] = queue.Queue()
        self._globals: dict[str, object] = {}
        self._stop_event = Event()
        self._ensure_safe_matplotlib_backend()

    @property
    def notebook_id(self) -> str:
        return self._notebook_id

    def enqueue(self, request: ExecutionRequest) -> None:
        self._queue.put(request)

    def shutdown(self) -> None:
        self._stop_event.set()
        self._queue.put(None)
        self.wait(2000)

    def run(self) -> None:  # type: ignore[override]
        while not self._stop_event.is_set():
            try:
                request = self._queue.get(timeout=0.1)
            except queue.Empty:
                continue

            if request is None:
                break

            self._run_request(request)

    # Internal helpers --------------------------------------------------
    def _run_request(self, request: ExecutionRequest) -> None:
        self.request_started.emit(request)

        stdout_io = io.StringIO()
        stderr_io = io.StringIO()
        style_ctx = self._matplotlib_style_context(request.plot_style)
        try:
            with style_ctx:
                with contextlib.redirect_stdout(stdout_io), contextlib.redirect_stderr(stderr_io):
                    exec(request.code, self._globals, self._globals)
            figures = self._collect_matplotlib_figures()
            result = ExecutionResult(
                notebook_id=request.notebook_id,
                cell_id=request.cell_id,
                execution_count=request.execution_count,
                stdout=stdout_io.getvalue(),
                stderr=stderr_io.getvalue(),
                globals_snapshot={},
                figures=figures,
            )
            self.request_finished.emit(result)
        except Exception as exc:  # noqa: BLE001 - surfacing user code errors
            tb = traceback.format_exc()
            figures = self._collect_matplotlib_figures()
            result = ExecutionResult(
                notebook_id=request.notebook_id,
                cell_id=request.cell_id,
                execution_count=request.execution_count,
                stdout=stdout_io.getvalue(),
                stderr=stderr_io.getvalue(),
                error=tb or str(exc),
                figures=figures,
            )
            self.request_failed.emit(result)

    def _ensure_safe_matplotlib_backend(self) -> None:
        """Prefer a headless backend when executing in worker threads."""
        if os.environ.get("MPLBACKEND"):
            return
        try:
            os.environ["MPLBACKEND"] = "Agg"
        except Exception:
            pass

    def _collect_matplotlib_figures(self) -> list[bytes]:
        try:
            import matplotlib  # noqa: F401
            from matplotlib import pyplot as plt
        except Exception:
            return []

        try:
            images: list[bytes] = []
            for fig_num in plt.get_fignums():
                fig = plt.figure(fig_num)
                if fig is None:
                    continue
                buffer = io.BytesIO()
                fig.savefig(buffer, format="png")
                images.append(buffer.getvalue())
            if images:
                plt.close('all')
            return images
        except Exception:
            return []

    def _matplotlib_style_context(self, style: dict[str, str] | None):
        if not style:
            return contextlib.nullcontext()
        try:
            import matplotlib

            return matplotlib.rc_context(style)
        except Exception:
            return contextlib.nullcontext()
