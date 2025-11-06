"""Main application window."""

from typing import Callable

from PySide6.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QLabel, QDockWidget, QListWidget
from PySide6.QtGui import QAction
from PySide6.QtCore import Qt

from ..constants.config import WindowConfig
from ..core.theme_manager import ThemeManager
from .menu_actions import file_actions, edit_actions, view_actions
from .menu_actions import notebook_actions, settings_actions, help_actions


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
        self._setup_window()
        self._setup_menubar()
        self._setup_statusbar()
        self._setup_notebook_sidebar()
        self._setup_ui()
        self.theme_manager.set_window(self)
        self.theme_manager.apply_theme(config.theme)
    
    def _setup_window(self) -> None:
        """Configure window properties."""
        self.setWindowTitle(self.config.title)
        self.resize(self.config.width, self.config.height)
        self.setMinimumSize(self.config.min_width, self.config.min_height)
    
    def _setup_statusbar(self) -> None:
        """Set up the status bar."""
        statusbar = self.statusBar()
        statusbar.showMessage("Ready")
    
    def _setup_notebook_sidebar(self) -> None:
        """Set up the notebook sidebar."""
        # Create dock widget
        self.notebook_dock = QDockWidget("Notebooks", self)
        self.notebook_dock.setAllowedAreas(
            Qt.DockWidgetArea.LeftDockWidgetArea | 
            Qt.DockWidgetArea.RightDockWidgetArea
        )
        
        # Create list widget for notebooks
        self.notebook_list = QListWidget()
        self.notebook_list.addItems([
            "Notebook 1",
            "Notebook 2",
            "Notebook 3",
            "Math Examples",
            "Physics Lab"
        ])
        
        # Set the list as dock widget content
        self.notebook_dock.setWidget(self.notebook_list)
        
        # Add to main window (default: left side)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, self.notebook_dock)
    
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
        edit_menu.addAction(self._create_action("Move Cell to Trash Bin", lambda: edit_actions.on_delete_cell(self)))
        
        # Insert cells menu
        insert_menu = menubar.addMenu("&Insert")
        insert_menu.addAction(self._create_action("Insert Text Cell", lambda: edit_actions.on_insert_text_cell(self)))
        insert_menu.addAction(self._create_action("Insert CAS Cell", lambda: edit_actions.on_insert_cas_cell(self)))
        insert_menu.addAction(self._create_action("Insert Python Cell", lambda: edit_actions.on_insert_python_cell(self)))

        # Notebooks menu
        notebooks_menu = menubar.addMenu("&Notebooks")
        notebooks_menu.addAction(self._create_action("New Notebook", lambda: notebook_actions.on_new_notebook(self)))
        notebooks_menu.addAction(self._create_action("Move Notebook to Trash Bin", lambda: notebook_actions.on_delete_notebook(self)))
        notebooks_menu.addSeparator()
        notebooks_menu.addAction(self._create_action("Notebook 1 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))
        notebooks_menu.addAction(self._create_action("Notebook 2 (placeholder)", lambda: notebook_actions.on_select_notebook(self)))

        # Notebooks menu
        notebooks_menu = menubar.addMenu("&Trash Bin")
        notebooks_menu.addAction(self._create_action("Open Notebook with individually deleted cells", lambda: notebook_actions.on_new_notebook(self)))
        notebooks_menu.addAction(self._create_action("Open 'Deleted Notebook 1 placeholder'", lambda: notebook_actions.on_delete_notebook(self)))
        notebooks_menu.addAction(self._create_action("Open 'Deleted Notebook 2 placeholder'", lambda: notebook_actions.on_delete_notebook(self)))

        # View menu
        view_menu = menubar.addMenu("&View")
        view_menu.addAction(self._create_action("Light Theme", self.theme_manager.set_light_theme))
        view_menu.addAction(self._create_action("Dark Theme", self.theme_manager.set_dark_theme))
        view_menu.addSeparator()
        view_menu.addAction(self._create_action("Toggle Notebooks Sidebar", self._toggle_notebook_sidebar))
        view_menu.addSeparator()
        view_menu.addAction(self._create_action("Normal Web View", lambda: view_actions.on_normal_view(self)))
        view_menu.addAction(self._create_action("A4 Paper View", lambda: view_actions.on_a4_view(self)))


        # Settings menu
        settings_menu = menubar.addMenu("&Settings")
        settings_menu.addAction(self._create_action("Font Size", lambda: settings_actions.on_font_size(self)))
        settings_menu.addAction(self._create_action("Font Family", lambda: settings_actions.on_font_family(self)))
        settings_menu.addAction(self._create_action("Precision", lambda: settings_actions.on_precision(self)))
        
        # Help menu
        help_menu = menubar.addMenu("&Help")
        help_menu.addAction(self._create_action("About Luna STEM Notebook", lambda: help_actions.on_about(self)))
        help_menu.addSeparator()
        help_menu.addAction(self._create_action("Markdown Help", lambda: help_actions.on_help_markdown(self)))
        help_menu.addAction(self._create_action("Python Help", lambda: help_actions.on_help_python(self)))
        help_menu.addAction(self._create_action("CAS Help", lambda: help_actions.on_help_cas(self)))
        help_menu.addAction(self._create_action("Geometry Help", lambda: help_actions.on_help_geometry(self)))
    
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
    
    def _toggle_notebook_sidebar(self) -> None:
        """Toggle the notebook sidebar visibility."""
        self.notebook_dock.setVisible(not self.notebook_dock.isVisible())
    
    def _setup_ui(self) -> None:
        """Set up the user interface."""
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        layout = QVBoxLayout()
        central_widget.setLayout(layout)
        
        # Header label
        self.header_label = QLabel("Luna STEM Notebook")
        self.header_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        font = self.header_label.font()
        font.setPointSize(24)
        font.setBold(True)
        self.header_label.setFont(font)
        
        # Add widgets to layout
        layout.addWidget(self.header_label)
        layout.addStretch()

