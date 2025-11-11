"""NotebookManager stub (post-MVP implementation planned).

Future responsibilities:
- Create/open/close notebooks
- Manage ordered cell_id list
- Save notebooks via DataStore

Minimal placeholder for now.
"""

from __future__ import annotations

class NotebookManager:
    """Planned API surface for notebook operations."""

    def __init__(self, store):  # store: DataStore (later)
        self._store = store

    # Methods will be added in later phases.
    pass
