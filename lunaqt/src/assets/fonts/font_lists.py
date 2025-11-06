"""Font lists configuration.

This module defines lists of fonts used in the application:
- System fonts: Standard fonts available on most operating systems
- Bundled fonts: Third-party fonts that ship with the application (in fonts/ subdirectories)
- Code fonts: Monospace fonts suitable for code editing

Note: Bundled fonts are loaded from the fonts/ directory at startup.
Only fonts listed here will appear in the font selection dropdowns.
"""

# Standard system fonts available on most platforms
SYSTEM_FONTS = [
    "Arial",
    "Helvetica",
    "Times New Roman",
    "Georgia",
]

# Third-party fonts bundled with the application
# These must match the font family names loaded from fonts/ subdirectories
BUNDLED_FONTS = [
    "OpenDyslexic",  # Accessible font for dyslexic readers
]

# System monospace/code fonts
SYSTEM_CODE_FONTS = [
    "Courier New",
    "Monaco",
    "Consolas",
    "Monospace"
]

# Bundled monospace/code fonts
# These must match the font family names loaded from fonts/ subdirectories
BUNDLED_CODE_FONTS = [
    # "OpenDyslexic Mono",  # Uncomment if mono variant is added
]


