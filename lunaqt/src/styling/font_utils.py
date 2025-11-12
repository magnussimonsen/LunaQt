"""Helpers for applying UI font settings across top-level widgets.

Keep GUI-specific font application in the GUI layer (not core),
so window/menu/status/header updates remain close to widgets.
"""
from PySide6.QtWidgets import QApplication, QMainWindow, QLabel, QWidget
from PySide6.QtGui import QFont


def _compute_header_point_size(ui_point_size: int) -> int:
    """Compute header label point size from UI font size.

    Roughly 2.2x of base UI font size, clamped 14–48 pt.
    """
    return max(14, min(48, int(ui_point_size * 2.2)))


def _set_font_recursive(widget: QWidget, font: QFont) -> None:
    """Apply font to widget and all descendant child widgets."""
    if widget is None:
        return
    widget.setFont(font)
    for child in widget.findChildren(QWidget):
        child.setFont(font)


def apply_ui_font(
    window: QMainWindow,
    font_family: str,
    size: int,
    header_label: QLabel | None = None,
) -> None:
    """Apply UI font to the application and common window elements.

    Args:
        window: Main window whose menubar/statusbar should be updated
        font_family: UI font family name
        size: UI font point size
        header_label: Optional header label to scale proportionally
    """
    app_font = QFont(font_family)
    app_font.setPointSize(size)
    # 1) Set application default font (affects widgets created after this)
    QApplication.instance().setFont(app_font)

    # 2) Also set on the existing main window to push the font to current children
    window.setFont(app_font)

    # Menubar and statusbar explicit sizes to avoid theme stylesheet overrides
    window.menuBar().setStyleSheet(
        f"QMenuBar {{ font-family: '{font_family}'; font-size: {size}pt; }}"
    )
    window.statusBar().setStyleSheet(
        f"QStatusBar {{ font-family: '{font_family}'; font-size: {size}pt; }}"
    )

    # Header label follows UI size proportionally
    if header_label is not None:
        header_font = QFont(font_family)
        header_font.setPointSize(_compute_header_point_size(size))
        header_font.setBold(True)
        header_label.setFont(header_font)

    # 3) Ensure settings dock subtree updates reactively if present
    #    Setting the font on the dock's content widget propagates to its children
    if hasattr(window, "settings_dock") and window.settings_dock is not None:
        # Apply to the dock itself (affects title area in some styles)
        window.settings_dock.setFont(app_font)
        # Apply to the dock content and all children to force immediate update
        dock_widget = window.settings_dock.widget()
        if dock_widget is not None:
            _set_font_recursive(dock_widget, app_font)
