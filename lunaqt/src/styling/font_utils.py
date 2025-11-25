"""Helpers for applying UI font settings across top-level widgets.

Keep GUI-specific font application in the GUI layer (not core),
so window/menu/status/header updates remain close to widgets.
"""
from PySide6.QtWidgets import QApplication, QMainWindow, QLabel, QWidget, QMenu
from PySide6.QtGui import QFont, QAction


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

    # Apply font directly to menubar and statusbar (without overriding QSS styling)
    menubar = window.menuBar()
    menubar.setFont(app_font)
    window.statusBar().setFont(app_font)

    # Ensure menus and their actions also inherit the font explicitly. When QSS is
    # active (especially in dark mode), Qt can stop propagating fonts implicitly.
    for action in menubar.actions():
        _apply_font_to_action(app_font, action)
        menu = action.menu()
        if menu is not None:
            menu.setFont(app_font)
            for sub_action in menu.actions():
                _apply_font_to_action(app_font, sub_action)

    # Apply font to menubar corner buttons if present
    if hasattr(window, 'settings_button') and window.settings_button is not None:
        window.settings_button.setFont(app_font)
    if hasattr(window, 'notebooks_button') and window.notebooks_button is not None:
        window.notebooks_button.setFont(app_font)

    # Header label follows UI size proportionally
    if header_label is not None:
        header_font = QFont(font_family)
        header_font.setPointSize(_compute_header_point_size(size))
        header_font.setBold(True)
        header_label.setFont(header_font)

    # 3) Ensure dock subtrees update reactively if present
    def _apply_font_to_dock(dock_attr: str) -> None:
        dock = getattr(window, dock_attr, None)
        if dock is None:
            return
        dock.setFont(app_font)
        dock_widget = dock.widget()
        if dock_widget is not None:
            _set_font_recursive(dock_widget, app_font)

    _apply_font_to_dock("settings_dock")
    _apply_font_to_dock("notebooks_dock")


def _apply_font_to_action(font: QFont, action: QAction | None) -> None:
    """Apply font to a QAction if supported.

    Some Qt styles ignore QAction fonts unless set directly, so we set them
    defensively whenever the UI font changes.
    """
    if action is None:
        return
    try:
        action.setFont(font)
    except AttributeError:
        pass
