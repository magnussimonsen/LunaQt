"""UUID generation utilities for cells and notebooks."""

from __future__ import annotations
import uuid


def generate_cell_id() -> str:
    """Generate a unique cell ID.
    
    Returns:
        A UUID4 string suitable for cell identification.
    """
    return str(uuid.uuid4())


def generate_notebook_id() -> str:
    """Generate a unique notebook ID.
    
    Returns:
        A UUID4 string suitable for notebook identification.
    """
    return str(uuid.uuid4())
