"""Centralized button styling for consistent appearance across the application.

This module provides reusable button style generators that maintain consistent
styling for different button states (normal, selected, disabled, running, etc.).
"""

from ..themes.colors import get_color
from ..constants.types import ThemeMode


def get_button_stylesheet(theme: ThemeMode, state: str = "normal") -> str:
    """Generate button stylesheet for a specific state.
    
    Args:
        theme: Current theme mode ('light' or 'dark')
        state: Button state - 'normal', 'selected', 'disabled', or 'running'
        
    Returns:
        Complete button stylesheet string
    """
    if state == "normal":
        return _get_normal_button_style(theme)
    elif state == "selected":
        return _get_selected_button_style(theme)
    elif state == "disabled":
        return _get_disabled_button_style(theme)
    elif state == "running":
        return _get_running_button_style(theme)
    else:
        raise ValueError(f"Unknown button state: {state}")


def _get_normal_button_style(theme: ThemeMode) -> str:
    """Get stylesheet for normal button state.
    
    Args:
        theme: Current theme mode
        
    Returns:
        Normal button stylesheet
    """
    button_bg = get_color("button_bg", theme)
    hover_bg = get_color("item_hover_bg", theme)
    pressed_bg = get_color("item_pressed_bg", theme)
    text_color = get_color("primary_text_color", theme)
    border_color = get_color("primary_border_color", theme)
    
    return f"""
        QPushButton {{
            background-color: {button_bg};
            color: {text_color};
            border: 1px solid {border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
        QPushButton:hover {{
            background-color: {hover_bg};
            color: {text_color};
        }}
        QPushButton:pressed {{
            background-color: {pressed_bg};
            color: {text_color};
        }}
    """


def _get_selected_button_style(theme: ThemeMode) -> str:
    """Get stylesheet for selected button state (e.g., settings panel open).
    
    Args:
        theme: Current theme mode
        
    Returns:
        Selected button stylesheet
    """
    bg_color = get_color("item_selected_bg", theme)
    text_color = get_color("primary_text_color", theme)
    border_color = get_color("primary_border_color", theme)

    return f"""
        QPushButton {{
            background-color: {bg_color};
            color: {text_color};
            border: 1px solid {border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
    """


def _get_disabled_button_style(theme: ThemeMode) -> str:
    """Get stylesheet for disabled button state (e.g., button not clickable).
    
    Args:
        theme: Current theme mode
        
    Returns:
        Disabled button stylesheet
    """
    button_bg = get_color("disabled_bg", theme)
    button_text_color = get_color("disabled_text_color", theme)
    button_border_color = get_color("secondary_border_color", theme)

    return f"""
        QPushButton {{
            background-color: {button_bg};
            color: {button_text_color};
            border: 1px solid {button_border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
        QPushButton:hover {{
            background-color: {button_bg};
        }}
    """


def _get_running_button_style(theme: ThemeMode) -> str:
    """Get stylesheet for running button state (e.g., code execution in progress).
    
    This uses disabled background to indicate the operation is in progress,
    but with normal text color to show it's an selected process.
    
    Args:
        theme: Current theme mode
        
    Returns:
        Running button stylesheet
    """
    button_disabled_bg = get_color("disabled_bg", theme)
    button_text_color = get_color("primary_text_color", theme)
    button_border_color = get_color("primary_border_color", theme)
    button_hover_bg = get_color("secondary_selected_bg", theme)

    return f"""
        QPushButton {{
            background-color: {button_disabled_bg};
            color: {button_text_color};
            border: 1px solid {button_border_color};
            padding: 6px 12px;
            border-radius: 4px;
        }}
        QPushButton:hover {{
            background-color: {button_hover_bg};
        }}
    """
