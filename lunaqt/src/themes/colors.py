"""Theme colors with light/dark mode support.

Color-first dictionary structure for easy maintenance.
Each color has light and dark variants.
"""

from ..constants.types import ThemeMode, ColorName

COLORS = {
    # Window colors
    "main_window_bg": {
        "light": "#FFFFFF",
        "dark": "#1E1E1E"
    },
    "secondary_bg": {
        "light": "#F5F5F5",
        "dark": "#2D2D2D"
    },
    
    # Text colors
    "primary_text": {
        "light": "#000000",
        "dark": "#FFFFFF"
    },
    "secondary_text": {
        "light": "#666666",
        "dark": "#AAAAAA"
    },
    
    # Accent colors
    "primary_accent": {
        "light": "#0066CC",
        "dark": "#4A9EFF"
    },
    "secondary_accent": {
        "light": "#00AA66",
        "dark": "#00DD88"
    },
    
    # Border colors
    "border": {
        "light": "#CCCCCC",
        "dark": "#444444"
    },
    
    # Button colors
    "button_bg": {
        "light": "#E0E0E0",
        "dark": "#3A3A3A"
    },
    "button_hover": {
        "light": "#D0D0D0",
        "dark": "#4A4A4A"
    }
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
