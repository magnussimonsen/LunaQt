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
    bg_color = get_color("main_window_bg", theme)
    secondary_bg = get_color("secondary_bg", theme)
    text_color = get_color("primary_text", theme)
    secondary_text = get_color("secondary_text", theme)
    accent_color = get_color("primary_accent", theme)
    border_color = get_color("border", theme)
    button_bg = get_color("button_bg", theme)
    button_hover = get_color("button_hover", theme)
    
    return f"""
        /* Main Window */
        QMainWindow {{
            background-color: {bg_color};
        }}
        
        /* Menu Bar */
        QMenuBar {{
            background-color: {bg_color};
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
            color: {bg_color};
            border-radius: 3px;
        }}
        
        QMenuBar::item:pressed {{
            background-color: {accent_color};
        }}
        
        /* Dropdown Menus */
        QMenu {{
            background-color: {bg_color};
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
            color: {bg_color};
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
            color: {bg_color};
        }}
    """
