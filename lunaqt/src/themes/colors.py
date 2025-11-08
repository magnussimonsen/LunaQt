"""Theme colors with light/dark mode support.

Color-first dictionary structure for easy maintenance.
Each color has light and dark variants.
"""

from ..constants.types import ThemeMode, ColorName

# Shades of gray (for reference light mode):
# #F5F5F5 (White smoke)
# #DEDEDE 
# #C7C7C7 
# #999999 
# #828282 
# #6B6B6B 

# Shades of gray (for reference dark mode)
#121212 (charcoal black, Background)
#E0E0E0 (light gray, Primary Text)
#B0B0B0 (medium gray, Secondary Text)
#444444 (dark gray, Borders/Dividers)
#888888 (soft gray, Accent )

COLORS = {
    # Background colors
    "main_window_bg": {
        "light": "#FFFFFF",
        "dark": "#121212"
    },
    "secondary_window_bg": { # Intended for menubars, sidebars, etc.
        "light": "whitesmoke",
        "dark": "#2D2D2D"
    },
    "third_window_bg": { # Intended for secondary menubars
        "light": "#F0F0F0",
        "dark": "#2D2D2D"
    },
    "fourth_window_bg": { # Intended A4-paper background color
        "light": "#E0E0E0",
        "dark": "#3A3A3A"
    },
    "main_menu_bg": {  # Redundant with secondary_window_bg
        "light": "whitesmoke",
        "dark": "#2A2A2A"
    },
    "secondary_menu_bg": { # Redundant with fourth_window_bg
        "light": "#F0F0F0",
        "dark": "#2D2D2D"
    },
    "disabled_bg": {
        "light": "#DDDDDD",
        "dark": "#3A3A3A"
    },
    "button_bg": {
        "light": "#DEDEDE",
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

    # Accent colors (Not needed)
    "primary_accent_color": {
        "light": "#000000",
        "dark": "whitesmoke"
    },
    "secondary_accent_color": {
        "light": "#00AA66",
        "dark": "#00DD88"
    },
    
    # Interactive item states (buttons, menu items, list items, etc.)
    "item_hover_bg": {  # Background when hovering over interactive items
        "light": "#C7C7C7",
        "dark": "#676767"
    },
    "item_pressed_bg": {  # Background when pressing down on items
        "light": "lightgreen",
        "dark": "forestgreen"
    },
    "item_selected_bg": {  # Background for selected items
        "light": "lightgreen",
        "dark": "forestgreen"
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
