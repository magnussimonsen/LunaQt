"""Main application entry point."""

import sys
from PySide6.QtWidgets import QApplication

from src.constants.config import WindowConfig, AppConfig
from src.constants.types import ThemeMode
from src.constants.window_size import (
    DEFAULT_WINDOW_WIDTH,
    DEFAULT_WINDOW_HEIGHT,
    MIN_WINDOW_WIDTH,
    MIN_WINDOW_HEIGHT
)
from src.constants.app_info import (
    APP_NAME,
    APP_HEADER_TEXT,
    APP_VERSION,
    AUTHOR,
    LICENSE,
    APP_DESCRIPTION
)
from src.gui.main_window import MainWindow
from src.core.font_manager import initialize_fonts


def create_window_config(theme: ThemeMode = "light") -> WindowConfig:
    """Create window configuration with defaults.
    
    Args:
        theme: Initial theme ('light' or 'dark')
        
    Returns:
        WindowConfig instance
    """
    return WindowConfig(
        width=DEFAULT_WINDOW_WIDTH,
        height=DEFAULT_WINDOW_HEIGHT,
        min_width=MIN_WINDOW_WIDTH,
        min_height=MIN_WINDOW_HEIGHT,
        title=APP_HEADER_TEXT,
        theme=theme
    )


def create_app_config() -> AppConfig:
    """Create application configuration.
    
    Returns:
        AppConfig instance
    """
    return AppConfig(
        name=APP_NAME,
        version=APP_VERSION,
        author=AUTHOR,
        license=LICENSE,
        description=APP_DESCRIPTION
    )


def main() -> int:
    """Run the application.
    
    Returns:
        Exit code
    """
    # Create Qt application
    app = QApplication(sys.argv)
    
    # Initialize custom fonts
    initialize_fonts()
    
    # Set application metadata
    app_config = create_app_config()
    app.setApplicationName(app_config.name)
    app.setApplicationVersion(app_config.version)
    app.setOrganizationName(app_config.author)
    
    # Create and show main window
    window_config = create_window_config(theme="light")
    window = MainWindow(window_config)
    window.show()
    
    # Run event loop
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
