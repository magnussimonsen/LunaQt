"""Theme management with QPalette-first approach."""

from typing import Optional
from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import Signal, QObject

from ..constants.types import ThemeMode
from ..styling.palette_builder import PaletteBuilder
from ..styling.base_qss import BaseQSS
from ..styling.semantic_colors import SemanticColors
from ..styling.notebook_qss import NotebookQSS


class ThemeManager(QObject):
    """
    Manages application theme using QPalette + minimal QSS.
    
    QPalette handles most colors automatically.
    QSS only for structural styling (borders, spacing, etc).
    """
    
    # Signal emitted when theme changes
    theme_changed = Signal(str)  # emits theme name ("light" or "dark")
    
    def __init__(self, initial_theme: ThemeMode = "light") -> None:
        """Initialize theme manager.
        
        Args:
            initial_theme: Starting theme mode
        """
        super().__init__()
        self._current_theme: ThemeMode = initial_theme
        self._window: Optional[QMainWindow] = None
    
    def set_window(self, window: QMainWindow) -> None:
        """Set the main window to apply themes to.
        
        Args:
            window: Main window instance
        """
        self._window = window
    
    @property
    def current_theme(self) -> ThemeMode:
        """Get current theme mode."""
        return self._current_theme
    
    def get_color(self, token: str) -> str:
        """
        Get semantic color for current theme.
        
        Args:
            token: Semantic token like 'surface.primary'
            
        Returns:
            Hex color string
        """
        return SemanticColors.get(self._current_theme, token)
    
    def apply_theme(self, theme: ThemeMode) -> None:
        """
        Apply theme to the application.
        
        Uses QPalette for colors, minimal QSS for structure.
        
        Args:
            theme: Theme mode to apply
        """
        if not self._window:
            raise RuntimeError("Window not set. Call set_window() first.")
        
        self._current_theme = theme
        
        # 1. Apply QPalette (handles most colors automatically)
        palette = PaletteBuilder.build(theme)
        QApplication.instance().setPalette(palette)
        
        # 2. Apply minimal QSS (only for structure/borders) + notebook-specific QSS
        base_qss = BaseQSS.get(theme)
        notebook_qss = NotebookQSS.get(theme)
        combined_qss = base_qss + "\n" + notebook_qss
        QApplication.instance().setStyleSheet(combined_qss)
        
        # 3. Emit signal for custom components
        self.theme_changed.emit(theme)
        
        # Force repaint
        self._window.update()
    
    def toggle_theme(self) -> None:
        """Toggle between light and dark theme."""
        new_theme: ThemeMode = "dark" if self._current_theme == "light" else "light"
        self.apply_theme(new_theme)
    
    def set_light_theme(self) -> None:
        """Switch to light theme."""
        self.apply_theme("light")
    
    def set_dark_theme(self) -> None:
        """Switch to dark theme."""
        self.apply_theme("dark")
