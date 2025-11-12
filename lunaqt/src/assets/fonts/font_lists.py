"""Font lists configuration.

This module defines lists of fonts used in the application.
All fonts listed here are bundled with the application and loaded at startup.
This ensures consistent typography across all platforms (Windows, macOS, Linux).

Font categories:
- UI fonts: For menus, buttons, and general interface elements
- Code fonts: Monospace fonts for code cells
"""

# UI fonts bundled with the application
# These are used for general interface elements
BUNDLED_FONTS = [
    "Inter",  # Modern, clean UI font - DEFAULT
    "Figtree",  # Clean geometric sans
    "Fira Code",  # Excellent code font with ligatures
    "Comic Neue",  # Playful UI/text option
    "OpenDyslexic",  # Accessible font for dyslexic readers
]

# Monospace/code fonts bundled with the application
# These are used for code and CAS cells
BUNDLED_CODE_FONTS = [
    "Inter",  # Modern, clean UI font - DEFAULT
    "Figtree",  # Clean geometric sans
    "Fira Code",  # Excellent code font with ligatures - DEFAULT
    "Comic Neue",  # Playful UI/text option
    "OpenDyslexic",  # Accessible font for dyslexic readers
]

# Default font selections
DEFAULT_UI_FONT = "Inter"
DEFAULT_CODE_FONT = "Fira Code"
DEFAULT_UI_FONT_SIZE = 12
DEFAULT_CODE_FONT_SIZE = 12
DEFAULT_TEXT_FONT_SIZE = 14



