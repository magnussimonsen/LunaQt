"""Color variable type definitions."""

from typing import Literal

# Color name keys from the COLORS dictionary
ColorName = Literal[
    "main_window_bg",
    "secondary_bg",
    "primary_text",
    "secondary_text",
    "primary_accent",
    "secondary_accent",
    "border",
    "button_bg",
    "button_hover"
]
