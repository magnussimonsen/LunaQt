"""Semantic color tokens for QPalette-based theming."""

from typing import Dict, Literal

ThemeMode = Literal["light", "dark"]


class SemanticColors:
    """
    Minimal, semantic color vocabulary.
    Each token defines both light and dark variants side-by-side.
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
        "surface.primary": {
            "light": "#f5f5f5",      # Main window background
            "dark": "#121212",
        },
        "surface.secondary": {
            "light": "#dcdcdc",      # Sidebar, menubar, panels
            "dark": "#1A1A1A",
        },
        "surface.tertiary": {
            "light": "#dcdcdc",      # Dropdowns 
            "dark": "#2A2A2A",
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
