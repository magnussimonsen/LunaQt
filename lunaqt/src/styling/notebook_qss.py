"""Notebook-specific QSS additions."""

from .semantic_colors import SemanticColors, ThemeMode


class NotebookQSS:
    """Additional QSS for notebook components."""
    
    @staticmethod
    def get(theme: ThemeMode) -> str:
        """Get notebook-specific QSS."""
        colors = SemanticColors.get_all(theme)
        
        return f"""
            /* ===== NOTEBOOK CELLS ===== */
            
            BaseCell {{
                border: 1px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 8px;
                margin: 4px 0px;
                /* Background from QPalette.Base */
                /* Text from QPalette.Text */
            }}
            
            /* ===== CELL GUTTER ===== */
            
            CellGutter {{
                background-color: {colors["surface.secondary"]};
                border-right: 10px solid {colors["border.subtle"]};
            }}
            
            CellGutter QLabel {{
                color: {colors["text.secondary"]};
                font-size: 10pt;
                padding: 2px 6px;
            }}
            
            BaseCell[selected="true"] {{
                border: 2px solid {colors["action.primary"]};
                /* Highlight background from QPalette.Highlight */
            }}
            
            BaseCell:hover {{
                border-color: {colors["action.hover"]};
            }}
            
            /* ===== CODE CELLS ===== */
            
            CodeCell QTextEdit {{
                background-color: {colors["code.background"]};
                color: {colors["code.text"]};
                border: none;
                /* Font family controlled by FontService via Python setFont() */
            }}
            
            /* ===== MARKDOWN CELLS ===== */
            
            MarkdownCell QTextEdit {{
                border: none;
                /* Background/text from QPalette */
            }}
            
            /* ===== CELL TOOLBARS ===== */
            
            /* Notebook toolbar container (menubar-like) */
            NotebookToolbarContainer {{
                background-color: {colors["surface.primary"]};
                border-bottom: 1px solid {colors["border.default"]};
                min-height: 32px;
                max-height: 40px;
            }}
            
            /* Individual toolbars inside container */
            BaseToolbar {{
                background-color: transparent;
                border: none;
                spacing: 6px;
                padding: 2px 8px;
            }}
            
            /* Toolbar buttons - more compact */
            BaseToolbar QPushButton {{
                padding: 3px 10px;
                min-height: 20px;
                max-height: 28px;
            }}
            
            /* Toolbar labels */
            BaseToolbar QLabel {{
                padding: 2px 4px;
            }}
        """
