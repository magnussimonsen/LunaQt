"""CellManager for cell CRUD operations and metadata.

Responsibilities:
- Create/update/delete cells
- Manage cell metadata and content
- Cell type conversions
- Coordinate with DataStore
"""

from __future__ import annotations
from datetime import datetime, timezone
from typing import Any

from .data_store import DataStore
from ..utils.id_generator import generate_cell_id


class CellManager:
    """Manages cell lifecycle and operations."""

    def __init__(self, store: DataStore) -> None:
        """Initialize cell manager.
        
        Args:
            store: DataStore instance for persistence.
        """
        self._store = store
    
    def create_cell(
        self,
        cell_type: str,
        content: str = "",
        metadata: dict[str, Any] | None = None
    ) -> str:
        """Create a new cell.
        
        Args:
            cell_type: Type of cell ('code', 'markdown', 'raw').
            content: Initial cell content.
            metadata: Optional metadata dict.
            
        Returns:
            The new cell's ID.
        """
        cell_id = generate_cell_id()
        now = datetime.now(timezone.utc).isoformat()
        
        if metadata is None:
            metadata = {}
        
        # Set defaults based on cell type
        if cell_type == "code":
            metadata.setdefault("execution_count", None)
            metadata.setdefault("language", "python")
        elif cell_type == "markdown":
            metadata.setdefault("language", "markdown")
        
        metadata.setdefault("collapsed", False)
        metadata.setdefault("tags", [])
        
        cell_data = {
            "cell_id": cell_id,
            "cell_type": cell_type,
            "content": content,
            "metadata": metadata,
            "outputs": [],
            "created_at": now,
            "modified_at": now,
            "schema_version": 1
        }
        
        self._store.save_cell(cell_data)
        return cell_id
    
    def get_cell(self, cell_id: str) -> dict[str, Any] | None:
        """Get cell data by ID.
        
        Args:
            cell_id: Cell identifier.
            
        Returns:
            Cell data dict or None if not found.
        """
        return self._store.load_cell(cell_id)
    
    def update_cell(
        self,
        cell_id: str,
        content: str | None = None,
        metadata: dict[str, Any] | None = None,
        outputs: list[dict[str, Any]] | None = None
    ) -> bool:
        """Update cell content, metadata, or outputs.
        
        Args:
            cell_id: Cell identifier.
            content: New content (None to keep current).
            metadata: Metadata updates (merged with existing).
            outputs: New outputs list (None to keep current).
            
        Returns:
            True if updated successfully.
        """
        cell_data = self._store.load_cell(cell_id)
        if not cell_data:
            return False
        
        if content is not None:
            cell_data["content"] = content
        
        if metadata is not None:
            cell_data["metadata"].update(metadata)
        
        if outputs is not None:
            cell_data["outputs"] = outputs
        
        # Update modified timestamp
        cell_data["modified_at"] = datetime.now(timezone.utc).isoformat()
        
        return self._store.save_cell(cell_data)
    
    def delete_cell(self, cell_id: str) -> bool:
        """Delete a cell.
        
        Args:
            cell_id: Cell identifier.
            
        Returns:
            True if deleted successfully.
        """
        return self._store.delete_cell(cell_id)
    
    def convert_cell_type(self, cell_id: str, new_type: str) -> bool:
        """Convert cell to a different type.
        
        Args:
            cell_id: Cell identifier.
            new_type: Target cell type ('code', 'markdown', 'raw').
            
        Returns:
            True if converted successfully.
        """
        cell_data = self._store.load_cell(cell_id)
        if not cell_data:
            return False
        
        old_type = cell_data["cell_type"]
        if old_type == new_type:
            return True  # Already the target type
        
        # Update cell type
        cell_data["cell_type"] = new_type
        
        # Adjust metadata for new type
        metadata = cell_data["metadata"]
        
        if new_type == "code":
            metadata["language"] = "python"
            metadata.setdefault("execution_count", None)
        elif new_type == "markdown":
            metadata["language"] = "markdown"
            # Remove code-specific metadata
            metadata.pop("execution_count", None)
        elif new_type == "raw":
            metadata["language"] = "raw"
            metadata.pop("execution_count", None)
        
        # Clear outputs when converting away from code
        if old_type == "code" and new_type != "code":
            cell_data["outputs"] = []
        
        cell_data["modified_at"] = datetime.now(timezone.utc).isoformat()
        
        return self._store.save_cell(cell_data)
    
    def duplicate_cell(self, cell_id: str) -> str | None:
        """Duplicate an existing cell.
        
        Args:
            cell_id: Cell identifier to duplicate.
            
        Returns:
            New cell ID or None if source not found.
        """
        cell_data = self._store.load_cell(cell_id)
        if not cell_data:
            return None
        
        # Create new cell with same content/type/metadata
        new_id = self.create_cell(
            cell_type=cell_data["cell_type"],
            content=cell_data["content"],
            metadata=cell_data["metadata"].copy()
        )
        
        return new_id

