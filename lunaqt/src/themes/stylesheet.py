"""Stylesheet generation for theme support."""

from ..constants.types import ThemeMode
from .colors import get_color


def get_stylesheet(theme: ThemeMode = "light") -> str:
    """Generate complete application stylesheet for given theme.
    
    Args:
        theme: Either 'light' or 'dark'
        
    Returns:
        Complete QSS stylesheet string
    """
    # Get colors for current theme
    main_window_bg = get_color("main_window_bg", theme)
    secondary_bg = get_color("secondary_window_bg", theme)
    text_color = get_color("primary_text_color", theme)
    secondary_text = get_color("secondary_text_color", theme)
    accent_color = get_color("primary_accent_color", theme)
    border_color = get_color("primary_border_color", theme)
    button_bg = get_color("button_bg", theme)
    button_hover = get_color("button_hover_bg", theme)
    
    return f"""
        /* Main Window */
        QMainWindow {{
            background-color: {main_window_bg};
        }}
        
        /* Menu Bar */
        QMenuBar {{
            background-color: {main_window_bg};
            color: {text_color};
            border-bottom: px solid {border_color};
            padding: 2px;
        }}
        
        QMenuBar::item {{
            background-color: transparent;
            padding: 4px 8px;
            margin: 2px;
        }}
        
        QMenuBar::item:selected {{
            background-color: {accent_color};
            color: {main_window_bg};
            border-radius: 3px;
        }}
        
        QMenuBar::item:pressed {{
            background-color: {accent_color};
        }}
        
        /* Dropdown Menus */
        QMenu {{
            background-color: {main_window_bg};
            color: {text_color};
            border: 1px solid {border_color};
            margin-top: 0px;
            padding: 4px;
        }}
        
        QMenu::item {{
            padding: 6px 20px;
            border-radius: 3px;
        }}
        
        QMenu::item:selected {{
            background-color: {accent_color};
            color: {main_window_bg};
        }}
        
        QMenu::separator {{
            height: 1px;
            background-color: {border_color};
            margin: 4px 8px;
        }}
        
        /* Labels */
        QLabel {{
            color: {text_color};
        }}
        
        /* Buttons */
        QPushButton {{
            background-color: {button_bg};
            color: {text_color};
            border: 1px solid {border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
        
        QPushButton:hover {{
            background-color: {button_hover};
        }}
        
        QPushButton:pressed {{
            background-color: {accent_color};
            color: {main_window_bg};
        }}
    """
