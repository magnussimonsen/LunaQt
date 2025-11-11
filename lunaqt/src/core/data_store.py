"""JSON persistence layer for notebooks and cells.

Responsibilities:
- Load/save cells and notebooks as JSON
- Atomic writes with temp files
- Path handling via QStandardPaths (OS-appropriate data directory)
"""

from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from PySide6.QtCore import QStandardPaths


class DataStore:
    """Handles notebook and cell persistence with atomic writes."""

    def __init__(self) -> None:
        """Initialize data store with OS-appropriate data directory."""
        self._data_dir = self._get_data_directory()
        self._notebooks_dir = self._data_dir / "notebooks"
        self._cells_dir = self._data_dir / "cells"
        
        # Ensure directories exist
        self._notebooks_dir.mkdir(parents=True, exist_ok=True)
        self._cells_dir.mkdir(parents=True, exist_ok=True)
    
    def _get_data_directory(self) -> Path:
        """Get OS-appropriate data directory using QStandardPaths.
        
        Returns:
            Path to LunaQt data directory (created if doesn't exist).
        """
        app_data = QStandardPaths.writableLocation(QStandardPaths.StandardLocation.AppDataLocation)
        data_path = Path(app_data) / "LunaQt"
        data_path.mkdir(parents=True, exist_ok=True)
        return data_path
    
    def load_notebook(self, notebook_id: str) -> dict[str, Any] | None:
        """Load notebook data by ID.
        
        Args:
            notebook_id: Unique notebook identifier.
            
        Returns:
            Notebook data dict or None if not found or invalid JSON.
        """
        file_path = self._notebooks_dir / f"{notebook_id}.json"
        if not file_path.exists():
            return None
        
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except (json.JSONDecodeError, OSError):
            return None
    
    def save_notebook(self, notebook_data: dict[str, Any]) -> bool:
        """Save notebook data atomically.
        
        Args:
            notebook_data: Must contain 'notebook_id' key.
            
        Returns:
            True if saved successfully, False otherwise.
        """
        notebook_id = notebook_data.get("notebook_id")
        if not notebook_id:
            return False
        
        file_path = self._notebooks_dir / f"{notebook_id}.json"
        return self._atomic_write(file_path, notebook_data)
    
    def load_cell(self, cell_id: str) -> dict[str, Any] | None:
        """Load cell data by ID.
        
        Args:
            cell_id: Unique cell identifier.
            
        Returns:
            Cell data dict or None if not found or invalid JSON.
        """
        file_path = self._cells_dir / f"{cell_id}.json"
        if not file_path.exists():
            return None
        
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except (json.JSONDecodeError, OSError):
            return None
    
    def save_cell(self, cell_data: dict[str, Any]) -> bool:
        """Save cell data atomically.
        
        Args:
            cell_data: Must contain 'cell_id' key.
            
        Returns:
            True if saved successfully, False otherwise.
        """
        cell_id = cell_data.get("cell_id")
        if not cell_id:
            return False
        
        file_path = self._cells_dir / f"{cell_id}.json"
        return self._atomic_write(file_path, cell_data)
    
    def delete_notebook(self, notebook_id: str) -> bool:
        """Delete a notebook file.
        
        Args:
            notebook_id: Unique notebook identifier.
            
        Returns:
            True if deleted, False if not found or error.
        """
        file_path = self._notebooks_dir / f"{notebook_id}.json"
        try:
            if file_path.exists():
                file_path.unlink()
                return True
            return False
        except OSError:
            return False
    
    def delete_cell(self, cell_id: str) -> bool:
        """Delete a cell file.
        
        Args:
            cell_id: Unique cell identifier.
            
        Returns:
            True if deleted, False if not found or error.
        """
        file_path = self._cells_dir / f"{cell_id}.json"
        try:
            if file_path.exists():
                file_path.unlink()
                return True
            return False
        except OSError:
            return False
    
    def list_notebooks(self) -> list[dict[str, Any]]:
        """List all notebooks with basic metadata.
        
        Returns:
            List of notebook data dicts (id, title, timestamps).
        """
        notebooks = []
        for file_path in self._notebooks_dir.glob("*.json"):
            data = self.load_notebook(file_path.stem)
            if data:
                notebooks.append(data)
        return notebooks
    
    def _atomic_write(self, file_path: Path, data: dict[str, Any]) -> bool:
        """Write JSON data atomically via temp file + rename.
        
        Args:
            file_path: Target file path.
            data: Data to serialize as JSON.
            
        Returns:
            True if written successfully, False otherwise.
        """
        temp_path = file_path.with_suffix(".tmp")
        try:
            with open(temp_path, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            
            # Atomic rename (on POSIX; on Windows replace is used)
            temp_path.replace(file_path)
            return True
        except (OSError, TypeError):
            # Clean up temp file on failure
            if temp_path.exists():
                temp_path.unlink()
            return False

