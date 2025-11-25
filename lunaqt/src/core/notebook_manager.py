"""NotebookManager for CRUD and cell ordering.

Responsibilities:
- Create/open/close notebooks
- Manage ordered cell_id list
- Add/remove/move cells within notebooks
- Save notebooks via DataStore
"""

from __future__ import annotations
from datetime import datetime, timezone
from typing import Any

from .data_store import DataStore
from ..utils.id_generator import generate_notebook_id


class NotebookManager:
    """Manages notebook lifecycle and cell ordering."""

    def __init__(self, store: DataStore) -> None:
        """Initialize notebook manager.
        
        Args:
            store: DataStore instance for persistence.
        """
        self._store = store
        self._active_notebook_id: str | None = None
    
    def create_notebook(self, title: str) -> str:
        """Create a new notebook.
        
        Args:
            title: Notebook title.
            
        Returns:
            The new notebook's ID.
        """
        notebook_id = generate_notebook_id()
        now = datetime.now(timezone.utc).isoformat()
        
        notebook_data = {
            "notebook_id": notebook_id,
            "title": title,
            "cell_ids": [],
            "metadata": {
                "kernel": "python3",
                "language": "python",
                "author": "",
                "tags": []
            },
            "created_at": now,
            "modified_at": now,
            "schema_version": 1
        }
        
        self._store.save_notebook(notebook_data)
        return notebook_id
    
    def open_notebook(self, notebook_id: str) -> dict[str, Any] | None:
        """Open a notebook by ID.
        
        Args:
            notebook_id: Notebook identifier.
            
        Returns:
            Notebook data dict or None if not found.
        """
        data = self._store.load_notebook(notebook_id)
        if data:
            self._active_notebook_id = notebook_id
        return data
    
    def close_notebook(self, notebook_id: str, save: bool = True) -> bool:
        """Close a notebook.
        
        Args:
            notebook_id: Notebook identifier.
            save: Whether to save before closing.
            
        Returns:
            True if closed successfully.
        """
        if save:
            data = self._store.load_notebook(notebook_id)
            if data:
                self.save_notebook(notebook_id)
        
        if self._active_notebook_id == notebook_id:
            self._active_notebook_id = None
        return True
    
    def save_notebook(self, notebook_id: str) -> bool:
        """Save a notebook.
        
        Args:
            notebook_id: Notebook identifier.
            
        Returns:
            True if saved successfully.
        """
        data = self._store.load_notebook(notebook_id)
        if not data:
            return False
        
        # Update modified timestamp
        data["modified_at"] = datetime.now(timezone.utc).isoformat()
        return self._store.save_notebook(data)
    
    def add_cell(self, notebook_id: str, cell_id: str, position: int = -1) -> bool:
        """Add a cell to a notebook at specified position.
        
        Args:
            notebook_id: Notebook identifier.
            cell_id: Cell identifier to add.
            position: Insert position (-1 for end).
            
        Returns:
            True if added successfully.
        """
        data = self._store.load_notebook(notebook_id)
        if not data:
            return False
        
        cell_ids = data.get("cell_ids", [])
        
        if position == -1:
            cell_ids.append(cell_id)
        else:
            cell_ids.insert(position, cell_id)
        
        data["cell_ids"] = cell_ids
        return self._store.save_notebook(data)
    
    def remove_cell(self, notebook_id: str, cell_id: str) -> bool:
        """Remove a cell from a notebook.
        
        Args:
            notebook_id: Notebook identifier.
            cell_id: Cell identifier to remove.
            
        Returns:
            True if removed successfully.
        """
        data = self._store.load_notebook(notebook_id)
        if not data:
            return False
        
        cell_ids = data.get("cell_ids", [])
        if cell_id in cell_ids:
            cell_ids.remove(cell_id)
            data["cell_ids"] = cell_ids
            return self._store.save_notebook(data)
        
        return False
    
    def move_cell(self, notebook_id: str, cell_id: str, new_position: int) -> bool:
        """Move a cell to a new position in the notebook.
        
        Args:
            notebook_id: Notebook identifier.
            cell_id: Cell identifier to move.
            new_position: Target position index.
            
        Returns:
            True if moved successfully.
        """
        data = self._store.load_notebook(notebook_id)
        if not data:
            return False
        
        cell_ids = data.get("cell_ids", [])
        if cell_id not in cell_ids:
            return False
        
        # Remove and reinsert
        cell_ids.remove(cell_id)
        cell_ids.insert(new_position, cell_id)
        
        data["cell_ids"] = cell_ids
        return self._store.save_notebook(data)
    
    def get_cell_order(self, notebook_id: str) -> list[str]:
        """Get ordered list of cell IDs for a notebook.
        
        Args:
            notebook_id: Notebook identifier.
            
        Returns:
            List of cell IDs in order.
        """
        data = self._store.load_notebook(notebook_id)
        if not data:
            return []
        return data.get("cell_ids", [])
    
    def get_active_notebook_id(self) -> str | None:
        """Get the currently active notebook ID.
        
        Returns:
            Active notebook ID or None.
        """
        return self._active_notebook_id
    
    def list_notebooks(self) -> list[dict[str, Any]]:
        """List all available notebooks.
        
        Returns:
            List of notebook data dicts.
        """
        return self._store.list_notebooks()

    def rename_notebook(self, notebook_id: str, new_title: str) -> bool:
        """Rename a notebook if it exists.

        Args:
            notebook_id: Notebook identifier.
            new_title: Desired title (non-empty).

        Returns:
            True if rename succeeded.
        """
        new_title = new_title.strip()
        if not new_title:
            return False
        data = self._store.load_notebook(notebook_id)
        if not data:
            return False
        data["title"] = new_title
        data["modified_at"] = datetime.now(timezone.utc).isoformat()
        return self._store.save_notebook(data)

    def delete_notebook(self, notebook_id: str) -> bool:
        """Delete a notebook and clear active reference if needed."""
        if not notebook_id:
            return False
        success = self._store.delete_notebook(notebook_id)
        if success and self._active_notebook_id == notebook_id:
            self._active_notebook_id = None
        return success

