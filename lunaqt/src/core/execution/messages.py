"""Shared dataclasses for notebook execution communication."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Optional


@dataclass(frozen=True)
class ExecutionRequest:
    """Represents a single code-cell execution request."""

    notebook_id: str
    cell_id: str
    code: str
    execution_count: Optional[int] = None
    plot_style: dict[str, str] | None = None


@dataclass(frozen=True)
class ExecutionResult:
    """Encapsulates the outcome of running a code cell."""

    notebook_id: str
    cell_id: str
    execution_count: Optional[int]
    stdout: str = ""
    stderr: str = ""
    error: Optional[str] = None
    globals_snapshot: dict[str, Any] = field(default_factory=dict)
    figures: list[bytes] = field(default_factory=list)

    @property
    def success(self) -> bool:
        return self.error is None
