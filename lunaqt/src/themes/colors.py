"""Theme colors with light/dark mode support.

Color-first dictionary structure for easy maintenance.
Each color has light and dark variants.
"""

from ..constants.types import ThemeMode, ColorName

COLORS = {
    # Background colors
    "main_window_bg": {
        "light": "#FFFFFF",
        "dark": "#1E1E1E"
    },
    "secondary_window_bg": {
        "light": "#F5F5F5",
        "dark": "#2D2D2D"
    },
    "main_menu_bg": {
        "light": "#EFEFEF",
        "dark": "#2A2A2A"
    },
    "secondary_menu_bg": {
        "light": "#F0F0F0",
        "dark": "#2D2D2D"
    },
    "disabled_bg": {
        "light": "#DDDDDD",
        "dark": "#3A3A3A"
    },
    "primary_active_bg": {
        "light": "#D0E4FF",
        "dark": "#3A5F9E"
    },
    "secondary_active_bg": {
        "light": "#E8F0FF",
        "dark": "#2E4A7E"
    },
    "button_bg": {
        "light": "#E0E0E0",
        "dark": "#3A3A3A"
    },
    "button_hover_bg": {
        "light": "#D0D0D0",
        "dark": "#4A4A4A"
    },

    # Text colors
    "primary_text_color": {
        "light": "#000000",
        "dark": "#FFFFFF"
    },
    "secondary_text_color": {
        "light": "#666666",
        "dark": "#AAAAAA"
    },
    "disabled_text_color": {
        "light": "#AAAAAA",
        "dark": "#555555"
    },
    
    # Border colors
    "primary_border_color": {
        "light": "#CCCCCC",
        "dark": "#444444"
    },
    "secondary_border_color": {
        "light": "#EEEEEE",
        "dark": "#333333"
    },

    # Accent colors
    "primary_accent_color": {
        "light": "#0066CC",
        "dark": "#4A9EFF"
    },
    "secondary_accent_color": {
        "light": "#00AA66",
        "dark": "#00DD88"
    },
}


def get_color(color_name: ColorName, theme: ThemeMode = "light") -> str:
    """Get color value for specific theme.
    
    Args:
        color_name: Name of the color from COLORS dict
        theme: Either 'light' or 'dark'
        
    Returns:
        Hex color string
        
    Raises:
        KeyError: If color_name or theme not found
    """
    return COLORS[color_name][theme]
