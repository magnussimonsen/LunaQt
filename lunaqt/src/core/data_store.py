"""JSON persistence layer (planned for post-MVP).

Responsibilities (future):
- Load/save cells and notebooks as JSON
- Atomic writes, basic caching
- Path handling via QStandardPaths/AppDirs

MVP: Placeholder to solidify imports and future structure.
"""

from __future__ import annotations

# Placeholder interfaces (kept minimal to avoid early coupling)
class DataStore:
    """Planned data store for notebook and cell persistence."""

    def __init__(self) -> None:
        pass
