"""Main application window."""

import logging
from typing import Callable

from PySide6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QLabel, QDockWidget, 
    QPushButton, QFormLayout, QComboBox, QSpinBox, QToolBar, QSizePolicy
)
from PySide6.QtWidgets import QApplication
from PySide6.QtGui import QAction, QFont
from pathlib import Path
import sys
from PySide6.QtCore import Qt, QSettings

from ..constants.config import WindowConfig
from ..core.theme_manager import ThemeManager
from ..core.font_manager import get_font_manager
from ..core.font_service import get_font_service
from ..core.data_store import DataStore
from ..core.notebook_manager import NotebookManager
from ..core.cell_manager import CellManager
from ..assets.fonts.font_lists import (
    BUNDLED_FONTS, BUNDLED_CODE_FONTS,
    DEFAULT_UI_FONT, DEFAULT_CODE_FONT,
    DEFAULT_UI_FONT_SIZE, DEFAULT_CODE_FONT_SIZE, DEFAULT_TEXT_FONT_SIZE
)
from .menu_actions import file_actions, edit_actions, view_actions
from .menu_actions import notebook_actions, settings_actions, help_actions

# Logger for MainWindow
logger = logging.getLogger(__name__)


class MainWindow(QMainWindow):
    """Main application window with theme support."""
    
    def __init__(self, config: WindowConfig) -> None:
        """Initialize main window.
        
        Args:
            config: Window configuration with size and theme settings
        """
        super().__init__()
        self.config = config
        self.theme_manager = ThemeManager(config.theme)
        # Core window and base font
        self._setup_window()
        self._set_default_font()

        # Build UI components (ensure central widget is set BEFORE dock widgets)
        self._setup_menubar()
        self._setup_statusbar()
        self._setup_ui()
        self._setup_settings_sidebar()

        # Theme hookup
        self.theme_manager.set_window(self)
        
        # Connect to theme changes for custom updates
        self.theme_manager.theme_changed.connect(self._on_theme_changed)
        
        # Apply initial theme
        self.theme_manager.apply_theme(config.theme)

        # Font service setup (other widgets can subscribe later)
        self.font_service = get_font_service()
        self.font_service.uiFontChanged.connect(self._on_ui_font_service_changed)
        self.font_service.textFontChanged.connect(self._on_text_font_service_changed)
        self.font_service.codeFontChanged.connect(self._on_code_font_service_changed)

        # Initialize service values then apply UI font
        self.font_service.set_ui_font(DEFAULT_UI_FONT, DEFAULT_UI_FONT_SIZE)
        self.font_service.set_text_font(DEFAULT_UI_FONT, DEFAULT_TEXT_FONT_SIZE)
        self.font_service.set_code_font(DEFAULT_CODE_FONT, DEFAULT_CODE_FONT_SIZE)
        self._apply_ui_font_to_widgets(DEFAULT_UI_FONT, DEFAULT_UI_FONT_SIZE)
        
        # Restore window geometry/state from previous session
        self._restore_window_state()
    
    def _restore_window_state(self) -> None:
        """Restore window geometry and dock widget state from QSettings."""
        settings = QSettings("LunaQt", "LunaQt")
        geometry = settings.value("window/geometry")
        state = settings.value("window/state")
        
        if geometry is not None:
            self.restoreGeometry(geometry)
        if state is not None:
            self.restoreState(state)
    
    def closeEvent(self, event) -> None:
        """Save window geometry/state before closing."""
        settings = QSettings("LunaQt", "LunaQt")
        settings.setValue("window/geometry", self.saveGeometry())
        settings.setValue("window/state", self.saveState())
        event.accept()
    
    def _setup_window(self) -> None:
        """Configure window properties."""
        self.setWindowTitle(self.config.title)
        self.resize(self.config.width, self.config.height)
        self.setMinimumSize(self.config.min_width, self.config.min_height)
    
    def _set_default_font(self) -> None:
        """Set the default application font to Inter."""
        default_font = QFont(DEFAULT_UI_FONT, DEFAULT_UI_FONT_SIZE)
        QApplication.instance().setFont(default_font)

    def _apply_ui_font_to_widgets(self, font_family: str, size: int) -> None:
        """Delegate font application to shared helper in gui.style.font_applier."""
        try:
            from .style.font_applier import apply_ui_font  # local import to avoid circulars
            apply_ui_font(self, font_family, size, self.header_label)
        except Exception as e:  # pragma: no cover - fallback path
            # Minimal fallback: still set application font so UI isn't broken
            app_font = QFont(font_family)
            app_font.setPointSize(size)
            QApplication.instance().setFont(app_font)
            self.statusBar().showMessage(f"Font apply helper failed: {e}")
    
    def _setup_statusbar(self) -> None:
        """Set up the status bar."""
        statusbar = self.statusBar()
        statusbar.showMessage("Ready")
    
    def _setup_settings_sidebar(self) -> None:
        """Set up the settings sidebar."""
        # Create dock widget for settings
        self.settings_dock = QDockWidget("Settings", self)

        # Lock to right side only
        self.settings_dock.setAllowedAreas(Qt.DockWidgetArea.RightDockWidgetArea)

        # Remove title bar to prevent undocking, but keep resizable
        # NoDockWidgetFeatures removes the title bar which prevents moving/floating
        # The dock widget will still be resizable by dragging its edge
        self.settings_dock.setFeatures(QDockWidget.DockWidgetFeature.NoDockWidgetFeatures)

        # Create settings panel content
        settings_widget = QWidget()
        settings_layout = QFormLayout()
        settings_widget.setLayout(settings_layout)
        # Help splitter sizing: prefer width but allow vertical expansion
        settings_widget.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)

        # Use only bundled fonts for consistent cross-platform experience
        all_fonts = BUNDLED_FONTS
        code_fonts = BUNDLED_CODE_FONTS

        # UI Font Family
        self.ui_font_family_combo = QComboBox()
        self.ui_font_family_combo.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToContents)
        self.ui_font_family_combo.addItems(all_fonts)
        self.ui_font_family_combo.setCurrentText(DEFAULT_UI_FONT)
        settings_layout.addRow("UI Font Family:", self.ui_font_family_combo)

        # Text Cell Font Family
        self.text_cell_font_family_combo = QComboBox()
        self.text_cell_font_family_combo.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToContents)
        self.text_cell_font_family_combo.addItems(all_fonts)
        self.text_cell_font_family_combo.setCurrentText(DEFAULT_UI_FONT)
        settings_layout.addRow("Text Cell Font Family:", self.text_cell_font_family_combo)

        # Code Cell Font Family
        self.code_cell_font_family_combo = QComboBox()
        self.code_cell_font_family_combo.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToContents)
        self.code_cell_font_family_combo.addItems(code_fonts)
        self.code_cell_font_family_combo.setCurrentText(DEFAULT_CODE_FONT)
        settings_layout.addRow("Code Cell Font Family:", self.code_cell_font_family_combo)

        # UI Font Size
        self.ui_font_size_spin = QSpinBox()
        self.ui_font_size_spin.setRange(8, 24)
        self.ui_font_size_spin.setValue(DEFAULT_UI_FONT_SIZE)
        settings_layout.addRow("UI Font Size:", self.ui_font_size_spin)

        # Text Cell Font Size
        self.text_cell_font_size_spin = QSpinBox()
        self.text_cell_font_size_spin.setRange(8, 32)
        self.text_cell_font_size_spin.setValue(DEFAULT_TEXT_FONT_SIZE)
        settings_layout.addRow("Text Cell Font Size:", self.text_cell_font_size_spin)

        # Code Cell Font Size
        self.code_cell_font_size_spin = QSpinBox()
        self.code_cell_font_size_spin.setRange(8, 24)
        self.code_cell_font_size_spin.setValue(DEFAULT_CODE_FONT_SIZE)
        settings_layout.addRow("Code Cell Font Size:", self.code_cell_font_size_spin)

        # Number Precision
        self.precision_spin = QSpinBox()
        self.precision_spin.setRange(1, 15)
        self.precision_spin.setValue(6)
        settings_layout.addRow("Number Precision:", self.precision_spin)

        # Connect signals to apply settings changes
        self.ui_font_family_combo.currentTextChanged.connect(self._on_ui_font_family_changed)
        self.ui_font_size_spin.valueChanged.connect(self._on_ui_font_size_changed)
        self.text_cell_font_family_combo.currentTextChanged.connect(self._on_text_cell_font_changed)
        self.text_cell_font_size_spin.valueChanged.connect(self._on_text_cell_size_changed)
        self.code_cell_font_family_combo.currentTextChanged.connect(self._on_code_cell_font_changed)
        self.code_cell_font_size_spin.valueChanged.connect(self._on_code_cell_size_changed)
        self.precision_spin.valueChanged.connect(self._on_precision_changed)

        # Set the widget as dock content
        self.settings_dock.setWidget(settings_widget)

        # Add to main window (default: right side, initially hidden)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, self.settings_dock)
        self.settings_dock.setObjectName("SettingsDock")
        # Set a reasonable minimum width to avoid snapping to 0 on first show
        self.settings_dock.setMinimumWidth(220)
        # Hide by default; width will be applied on first show via resizeDocks
        self.settings_dock.hide()
    
    def _setup_menubar(self) -> None:
        """Set up the menu bar with all menus and actions."""
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("&File")
        file_menu.addAction(self._create_action("New File", lambda: file_actions.on_new_file(self)))
        file_menu.addAction(self._create_action("Open File", lambda: file_actions.on_open_file(self)))
        file_menu.addSeparator()
        file_menu.addAction(self._create_action("Save File", lambda: file_actions.on_save_file(self)))
        file_menu.addAction(self._create_action("Save File As...", lambda: file_actions.on_save_file_as(self)))
        file_menu.addSeparator()
        file_menu.addAction(self._create_action("Export as PDF", lambda: file_actions.on_export_pdf(self)))
        file_menu.addSeparator()
        file_menu.addAction(self._create_action("About Luna STEM Notebook", lambda: file_actions.on_about(self)))
        file_menu.addSeparator()
        file_menu.addAction(self._create_action("Quit", lambda: file_actions.on_quit(self)))
        
        # Edit menu
        edit_menu = menubar.addMenu("&Edit")
        edit_menu.addAction(self._create_action("Move Cell Up", lambda: edit_actions.on_move_cell_up(self)))
        edit_menu.addAction(self._create_action("Move Cell Down", lambda: edit_actions.on_move_cell_down(self)))
        edit_menu.addSeparator()
        edit_menu.addAction(self._create_action("Delete Cell", lambda: edit_actions.on_delete_cell(self)))
        edit_menu.addAction(self._create_action("Delete Notebook", lambda: notebook_actions.on_delete_notebook(self)))

        # Insert cells menu
        insert_menu = menubar.addMenu("&Insert")
        insert_menu.addAction(self._create_action("Insert Text Cell", lambda: edit_actions.on_insert_text_cell(self)))
        insert_menu.addAction(self._create_action("Insert CAS Cell", lambda: edit_actions.on_insert_cas_cell(self)))
        insert_menu.addAction(self._create_action("Insert Python Cell", lambda: edit_actions.on_insert_python_cell(self)))

        # Notebooks menu
        notebooks_menu = menubar.addMenu("&Notebooks")
        notebooks_menu.addAction(self._create_action("Create New Notebook", lambda: notebook_actions.on_new_notebook(self)))
        notebooks_menu.addSeparator()
        notebooks_menu.addAction(self._create_action("Notebook 1 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 3 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 4 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        
        # View menu
        view_menu = menubar.addMenu("&View")
        view_menu.addAction(self._create_action("Light Theme", self.theme_manager.set_light_theme))
        view_menu.addAction(self._create_action("Dark Theme", self.theme_manager.set_dark_theme))
        view_menu.addSeparator()
        view_menu.addAction(self._create_action("Normal Web View", lambda: view_actions.on_normal_view(self)))
        view_menu.addAction(self._create_action("A4 Paper View", lambda: view_actions.on_a4_view(self)))

        # Help menu
        help_menu = menubar.addMenu("&Help")
        help_menu.addAction(self._create_action("About Luna STEM Notebook", lambda: help_actions.on_about(self)))
        help_menu.addSeparator()
        help_menu.addAction(self._create_action("Markdown Help", lambda: help_actions.on_help_markdown(self)))
        help_menu.addAction(self._create_action("Python Help", lambda: help_actions.on_help_python(self)))
        help_menu.addAction(self._create_action("CAS Help", lambda: help_actions.on_help_cas(self)))
        help_menu.addAction(self._create_action("Geometry Help", lambda: help_actions.on_help_geometry(self)))
        
        # Add settings button to the corner of the menu bar
        settings_button = QPushButton("Settings")
        settings_button.setCheckable(True)
        settings_button.setChecked(False)
        settings_button.clicked.connect(self._toggle_settings_sidebar)
        menubar.setCornerWidget(settings_button, Qt.Corner.TopRightCorner)
        self.settings_button = settings_button
    
    def _create_action(self, text: str, slot: Callable[[], None]) -> QAction:
        """Create a menu action.
        
        Args:
            text: Action text
            slot: Function to call when action is triggered
            
        Returns:
            QAction instance
        """
        action = QAction(text, self)
        action.triggered.connect(slot)
        return action
    
    def _toggle_settings_sidebar(self) -> None:
        """Toggle the settings sidebar visibility."""
        is_visible = not self.settings_dock.isVisible()
        self.settings_dock.setVisible(is_visible)
        # If showing, set an initial reasonable width to avoid snapping to edge
        if is_visible:
            try:
                # Request ~300px width for the right dock area
                self.resizeDocks([self.settings_dock], [300], Qt.Orientation.Horizontal)
            except Exception:
                pass
        # Update the checked state of the settings button
        self.settings_button.setChecked(is_visible)
    
    def _on_theme_changed(self, theme: str) -> None:
        """Handle theme changes for custom components.
        
        Most widgets update automatically via QPalette.
        No special handling needed - QPushButton:checked handles settings button.
        
        Args:
            theme: The new theme name ("light" or "dark")
        """
        pass
    
    def _on_ui_font_family_changed(self, font_family: str) -> None:
        """Handle UI font family change."""
        if not font_family:
            return
        size = self.ui_font_size_spin.value()
        self.font_service.set_ui_font(font_family, size)
        logger.debug("UI font family changed to: %s", font_family)
    
    def _on_ui_font_size_changed(self, size: int) -> None:
        """Handle UI font size change."""
        font_family = self.ui_font_family_combo.currentText()
        if not font_family:
            return
        self.font_service.set_ui_font(font_family, size)
        logger.debug("UI font size changed to: %s", size)
    
    def _on_text_cell_font_changed(self, font_family: str) -> None:
        """Handle text cell font family change."""
        # This will be used when text cells are implemented
        size = self.text_cell_font_size_spin.value()
        self.font_service.set_text_font(font_family, size)
        logger.debug("Text cell font family changed to: %s", font_family)
    
    def _on_text_cell_size_changed(self, size: int) -> None:
        """Handle text cell font size change."""
        # This will be used when text cells are implemented
        family = self.text_cell_font_family_combo.currentText()
        if family:
            self.font_service.set_text_font(family, size)
        logger.debug("Text cell font size changed to: %s", size)
    
    def _on_code_cell_font_changed(self, font_family: str) -> None:
        """Handle code cell font family change."""
        # This will be used when code cells are implemented
        size = self.code_cell_font_size_spin.value()
        self.font_service.set_code_font(font_family, size)
        logger.debug("Code cell font family changed to: %s", font_family)
    
    def _on_code_cell_size_changed(self, size: int) -> None:
        """Handle code cell font size change."""
        # This will be used when code cells are implemented
        family = self.code_cell_font_family_combo.currentText()
        if family:
            self.font_service.set_code_font(family, size)
        logger.debug("Code cell font size changed to: %s", size)

    # Service signal callbacks (could be extended to update existing cell widgets later)
    def _on_ui_font_service_changed(self, family: str, size: int) -> None:
        self._apply_ui_font_to_widgets(family, size)
        # Also resize header icon proportionally to UI font size to avoid excessive width hints
        try:
            from .style.set_app_icon import resize_header_icon
            from .style.font_applier import _compute_header_point_size
            new_h = max(24, min(96, _compute_header_point_size(size) * 2))
            resize_header_icon(self.header_icon_label, new_h)
        except Exception:
            pass

    def _on_text_font_service_changed(self, family: str, size: int) -> None:  # placeholder
        pass

    def _on_code_font_service_changed(self, family: str, size: int) -> None:  # placeholder
        pass
    
    def _on_precision_changed(self, precision: int) -> None:
        """Handle number precision change."""
        # This will be used when numeric output is implemented
        logger.debug("Number precision changed to: %s", precision)
    
    def _setup_ui(self) -> None:
        """Set up the user interface."""
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        layout = QVBoxLayout()
        central_widget.setLayout(layout)
        
        # Header row: app icon + title centered (via helper)
        from .style.set_app_icon import create_header_widget, build_window_qicon, resize_header_icon
        header_row, self.header_label, self.header_icon_label = create_header_widget("LunaQt Notebook")
        
        # Add header
        layout.addWidget(header_row)
        
        # Add NotebookView as main content area
        from .notebook.notebook_view import NotebookView
        self.notebook_view = NotebookView()
        layout.addWidget(self.notebook_view)

        # Initialize data store and managers, load or create default notebook
        try:
            self._data_store = DataStore()
            self._notebook_manager = NotebookManager(self._data_store)
            self._cell_manager = CellManager(self._data_store)

            notebooks = self._notebook_manager.list_notebooks()
            if notebooks:
                active_id = notebooks[0].get("notebook_id")
            else:
                active_id = self._notebook_manager.create_notebook("Untitled Notebook")
            # Open and load into view
            self._notebook_manager.open_notebook(active_id)
            self.notebook_view.set_managers(self._notebook_manager, self._cell_manager)
            self.notebook_view.set_active_notebook(active_id)
        except Exception as e:
            # Non-fatal: show message in status bar
            self.statusBar().showMessage(f"Notebook init failed: {e}")

        # Ensure window icon is set as well
        icon_qicon = build_window_qicon()
        if icon_qicon is not None:
            self.setWindowIcon(icon_qicon)

