"""Semantic color tokens for QPalette-based theming."""

from typing import Dict, Literal

ThemeMode = Literal["light", "dark"]


_SURFACE_FIRST: Dict[ThemeMode, str] = {
    "light": "#ffffff",      # Main window background
    "dark": "#121212",
}

_SURFACE_SECOND: Dict[ThemeMode, str] = {
    "light": "#ffffff",      # Menubar/statusbar background
    "dark": "#1A1A1A",
}

_SURFACE_THIRD: Dict[ThemeMode, str] = {
    "light": "#dcdcdc",      # Toolbars, side panels
    "dark": "#2A2A2A",
}

_SURFACE_FOURTH: Dict[ThemeMode, str] = {
    "light": "#cfcfcf",      # Cell input background
    "dark": "#3A3A3A",
}


class SemanticColors:
    """
    Semantic color vocabulary.
    Each token defines both light and dark variants side-by-side.
    The core idea is that developers can change the colors in one place
    without touching the rest of the codebase.
    """
    
    # Define semantic roles with light/dark pairs
    TOKENS: Dict[str, Dict[ThemeMode, str]] = {
        # === DEBUG COLOR 1 AND 2 FOR DEVELOPMENT ===
        "debug.color1": {
            "light": "#FF00FF",
            "dark": "#FF00FF",
        },
        "debug.color2": {
            "light": "#FF0000",
            "dark": "#FF0000",
        },

        # === SURFACE COLORS (backgrounds) ===
        "surface.first": _SURFACE_FIRST,
        "surface.primary": _SURFACE_FIRST,  # Legacy alias
        "surface.second": _SURFACE_SECOND,
        "surface.secondary": _SURFACE_SECOND,  # Legacy alias
        "surface.third": _SURFACE_THIRD,
        "surface.tertiary": _SURFACE_THIRD,  # Legacy alias
        "surface.fourth": _SURFACE_FOURTH,
        "surface.quaternary": _SURFACE_FOURTH,  # Legacy alias
        "surface.fifth": {
            "light": "#e8e8e8",      # Cell output backgrounds
            "dark": "#2E2E2E",
        },
        "surface.elevated": {
            "light": "#ababab",      # Dialogs, popups
            "dark": "#4A4A4A",
        },
        # === TEXT COLORS ===
        "text.primary": {
            "light": "#000000",      # Primary text
            "dark": "#f5f5f5",
        },
        "text.secondary": {
            "light": "#3B3B3B",      # Muted text
            "dark": "#dcdcdc",
        },
        "text.disabled": {
            "light": "#7a7a7a", # Disabled state
            "dark": "#ababab",
        },
        "text.inverted": {
            "light": "#f5f5f5",      # Text on dark backgrounds
            "dark": "#f5f5f5",
        },
        
        # === INTERACTIVE COLORS ===
        "action.primary": {
            "light": "#0078D4",      # Selections, focus, primary actions
            "dark": "#0E639C",
        },
        "action.hover": {
            "light": "#1A8CFF",      # Hover state
            "dark": "#1177BB",
        },
        "action.pressed": {
            "light": "#005A9E",      # Active/pressed state
            "dark": "#094771",
        },
        "action.disabled": {
            "light": "#DDDDDD",      # Disabled buttons
            "dark": "#4A4A4A",
        },
        
        # === BORDER COLORS ===
        "border.default": {
            "light": "#CCCCCC",      # Standard borders
            "dark": "#444444",
        },
        "border.subtle": {
            "light": "#EEEEEE",      # Subtle dividers
            "dark": "#333333",
        },
        "border.focus": {
            "light": "#0078D4",      # Focus indicators
            "dark": "#0E639C",
        },
        
        # === STATUS COLORS ===
        "status.success": {
            "light": "#107C10",
            "dark": "#4EC9B0",
        },
        "status.warning": {
            "light": "#CA5010",
            "dark": "#CE9178",
        },
        "status.error": {
            "light": "#D13438",
            "dark": "#F48771",
        },
        "status.info": {
            "light": "#0078D4",
            "dark": "#0E639C",
        },
        
        # === CODE/NOTEBOOK SPECIFIC ===
        "code.background": {
            "light": "#F8F8F8",
            "dark": "#1E1E1E",
        },
        "code.text": {
            "light": "#1E1E1E",
            "dark": "#D4D4D4",
        },
        "code.selection": {
            "light": "#ADD6FF",
            "dark": "#264F78",
        },
    }
    
    @classmethod
    def get(cls, theme: ThemeMode, token: str) -> str:
        """
        Get color by semantic token.
        
        Args:
            theme: 'light' or 'dark'
            token: Semantic token like 'surface.primary'
            
        Returns:
            Hex color string
        """
        return cls.TOKENS[token][theme]
    
    @classmethod
    def get_all(cls, theme: ThemeMode) -> Dict[str, str]:
        """Get all color tokens for a theme."""
        return {token: colors[theme] for token, colors in cls.TOKENS.items()}
