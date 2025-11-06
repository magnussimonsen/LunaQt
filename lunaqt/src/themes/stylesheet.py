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
    secondary_window_bg = get_color("secondary_window_bg", theme)
    third_window_bg = get_color("third_window_bg", theme)
    fourth_window_bg = get_color("fourth_window_bg", theme)
    disabled_bg = get_color("disabled_bg", theme)
    button_bg = get_color("button_bg", theme)
    
    primary_text_color = get_color("primary_text_color", theme)
    secondary_text_color = get_color("secondary_text_color", theme)
    disabled_text_color = get_color("disabled_text_color", theme)
    
    primary_border_color = get_color("primary_border_color", theme)
    secondary_border_color = get_color("secondary_border_color", theme)
    
    primary_accent_color = get_color("primary_accent_color", theme)
    secondary_accent_color = get_color("secondary_accent_color", theme)
    
    # Interactive item states
    item_hover_bg = get_color("item_hover_bg", theme)
    item_pressed_bg = get_color("item_pressed_bg", theme)
    item_selected_bg = get_color("item_selected_bg", theme)
    
    return f"""
        /* Main Window */
        QMainWindow {{
            background-color: {main_window_bg};
        }}
        
        /* Menu Bar */
        QMenuBar {{
            background-color: {secondary_window_bg};
            color: {primary_text_color};
            border: 0px solid {primary_border_color};
            padding: 0px;
        }}
        
        QMenuBar::item {{
            background-color: transparent;
            padding: 4px 8px;
            margin: 2px;
        }}
        
        QMenuBar::item:selected {{
            background-color: {item_hover_bg};
            color: {primary_text_color};
            border-radius: 3px;
        }}
        
        QMenuBar::item:pressed {{
            background-color: {item_pressed_bg};
            color: {primary_text_color};
        }}
        
        /* Dropdown Menus */
        QMenu {{
            background-color: {main_window_bg};
            color: {primary_text_color};
            border: 1px solid {primary_border_color};
            margin-top: 0px;
            padding: 4px;
        }}
        
        QMenu::item {{
            padding: 6px 20px;
            border-radius: 3px;
        }}
        
        QMenu::item:selected {{
            background-color: {item_hover_bg};
            color: {primary_text_color};
        }}
        
        QMenu::separator {{
            height: 1px;
            background-color: {primary_border_color};
            margin: 4px 8px;
        }}
        
        /* Settings Dock Widget */
        QDockWidget {{
            background-color: {secondary_window_bg};
            color: {primary_text_color};
            border: 1px solid {primary_border_color};
        }}
        
        QDockWidget::title {{
            background-color: {secondary_window_bg};
            color: {primary_text_color};
            padding: 4px;
            text-align: left;
        }}
        
        QDockWidget > QWidget {{
            background-color: {secondary_window_bg};
        }}
        
        /* Labels */
        QLabel {{
            color: {primary_text_color};
        }}
        
        /* Buttons */
        /* Buttons */
        QPushButton {{
            background-color: {button_bg};
            color: {primary_text_color};
            border: 1px solid {primary_border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
        
        QPushButton:hover {{
            background-color: {item_hover_bg};
            color: {primary_text_color};

        }}
        
        QPushButton:pressed {{
            background-color: {item_pressed_bg};
            color: {primary_text_color};

        }}

    """
