"""Theme management - core business logic for theme switching.

This module handles theme state and application logic,
independent of GUI implementation.
"""

from typing import Optional
from PySide6.QtWidgets import QMainWindow

from ..constants.types import ThemeMode
from ..themes.colors import get_color
from ..themes.stylesheet import get_stylesheet


class ThemeManager:
    """Manages application theme state and switching."""
    
    def __init__(self, initial_theme: ThemeMode = "light") -> None:
        """Initialize theme manager.
        
        Args:
            initial_theme: Starting theme mode
        """
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
    
    def apply_theme(self, theme: ThemeMode) -> None:
        """Apply theme to the window.
        
        Args:
            theme: Theme mode to apply
        """
        if not self._window:
            raise RuntimeError("Window not set. Call set_window() first.")
        
        self._current_theme = theme
        
        # Apply complete stylesheet
        stylesheet = get_stylesheet(theme)
        self._window.setStyleSheet(stylesheet)
        
        # Apply special styling to header if it exists
        if hasattr(self._window, 'header_label'):
            accent_color = get_color("primary_accent_color", theme)
            self._window.header_label.setStyleSheet(f"color: {accent_color};")
    
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
