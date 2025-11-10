"""QPalette builder for semantic theme application."""

from PySide6.QtGui import QPalette, QColor
from .semantic_colors import SemanticColors, ThemeMode


class PaletteBuilder:
    """Builds QPalette from semantic color tokens."""
    
    @staticmethod
    def build(theme: ThemeMode) -> QPalette:
        """
        Build QPalette for the given theme.
        
        Maps semantic tokens to QPalette color roles.
        Qt will automatically apply these to widgets.
        """
        palette = QPalette()
        colors = SemanticColors.get_all(theme)
        
        # === Window & Widget Backgrounds ===
        palette.setColor(QPalette.Window, QColor(colors["surface.primary"]))
        palette.setColor(QPalette.Base, QColor(colors["surface.secondary"]))
        palette.setColor(QPalette.AlternateBase, QColor(colors["surface.tertiary"]))
        palette.setColor(QPalette.ToolTipBase, QColor(colors["surface.elevated"]))
        
        # === Text Colors ===
        palette.setColor(QPalette.WindowText, QColor(colors["text.primary"]))
        palette.setColor(QPalette.Text, QColor(colors["text.primary"]))
        palette.setColor(QPalette.ToolTipText, QColor(colors["text.primary"]))
        palette.setColor(QPalette.PlaceholderText, QColor(colors["text.secondary"]))
        
        # === Button Colors ===
        palette.setColor(QPalette.Button, QColor(colors["action.disabled"]))
        palette.setColor(QPalette.ButtonText, QColor(colors["text.primary"]))
        
        # === Selection/Highlight Colors ===
        palette.setColor(QPalette.Highlight, QColor(colors["action.primary"]))
        palette.setColor(QPalette.HighlightedText, QColor(colors["text.inverted"]))
        
        # === Link Colors ===
        palette.setColor(QPalette.Link, QColor(colors["action.primary"]))
        palette.setColor(QPalette.LinkVisited, QColor(colors["action.pressed"]))
        
        # === Disabled State (for all color groups) ===
        palette.setColor(QPalette.Disabled, QPalette.WindowText, 
                        QColor(colors["text.disabled"]))
        palette.setColor(QPalette.Disabled, QPalette.Text, 
                        QColor(colors["text.disabled"]))
        palette.setColor(QPalette.Disabled, QPalette.ButtonText, 
                        QColor(colors["text.disabled"]))
        palette.setColor(QPalette.Disabled, QPalette.Button, 
                        QColor(colors["action.disabled"]))
        
        return palette
