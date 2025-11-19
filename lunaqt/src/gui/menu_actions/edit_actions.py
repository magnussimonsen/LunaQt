"""Edit menu action handlers.

Wire menubar actions to NotebookView operations.
"""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from PySide6.QtWidgets import QMainWindow


def _nv(window: "QMainWindow"):
    return getattr(window, "notebook_view", None)


def on_run_selected_cell(window: "QMainWindow") -> None:
    nv = _nv(window)
    if nv:
        nv.run_selected_cell()


def on_move_cell_up(window: "QMainWindow") -> None:
    nv = _nv(window)
    if nv:
        nv.move_selected_up()


def on_move_cell_down(window: "QMainWindow") -> None:
    nv = _nv(window)
    if nv:
        nv.move_selected_down()


def on_delete_cell(window: "QMainWindow") -> None:
    nv = _nv(window)
    if nv:
        nv.delete_selected()


def on_insert_text_cell(window: "QMainWindow") -> None:
    """Insert a Markdown (text) cell below selection or at end if none."""
    nv = _nv(window)
    if not nv:
        return
    idx = nv.get_selected_index()
    if idx is None:
        nv.insert_at_end("markdown")
    else:
        nv.insert_below("markdown")


def on_insert_cas_cell(window: "QMainWindow") -> None:
    """Insert a CAS cell (mapped to code for MVP) below selection or at end."""
    nv = _nv(window)
    if not nv:
        return
    idx = nv.get_selected_index()
    target_type = "code"  # TODO: introduce dedicated CAS type later
    if idx is None:
        nv.insert_at_end(target_type)
    else:
        nv.insert_below(target_type)


def on_insert_python_cell(window: "QMainWindow") -> None:
    """Insert a Python (code) cell below selection or at end if none."""
    nv = _nv(window)
    if not nv:
        return
    idx = nv.get_selected_index()
    if idx is None:
        nv.insert_at_end("code")
    else:
        nv.insert_below("code")
