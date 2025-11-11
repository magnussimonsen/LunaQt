"""CellManager stub (post-MVP implementation planned).

Future responsibilities:
- Create/update/delete cells
- Manage metadata and outputs linkage
- Coordinate with DataStore

Keep minimal now to avoid premature complexity.
"""

from __future__ import annotations

class CellManager:
    """Planned API surface for cell operations."""

    def __init__(self, store):  # store: DataStore (later)
        self._store = store

    # Methods will be added in later phases.
    pass
