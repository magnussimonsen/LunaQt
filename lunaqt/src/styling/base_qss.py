"""Minimal QSS for things QPalette cannot handle."""

from .semantic_colors import SemanticColors, ThemeMode


class BaseQSS:
    """
    Generates minimal QSS for structural styling only.
    
    QPalette handles most colors. QSS only for:
    - Borders, spacing, margins, padding
    - Border radius, shadows
    - Scrollbar styling (Qt doesn't palette these well)
    - Custom widget-specific overrides
    """
    
    @staticmethod
    def get(theme: ThemeMode) -> str:
        """Get minimal QSS for structural styling."""
        colors = SemanticColors.get_all(theme)
        # Resolve themed arrow icons for spin boxes
        up_icon = "src/styling/theme_icons/up_light.svg" if theme == "light" else "src/styling/theme_icons/up_dark.svg"
        down_icon = "src/styling/theme_icons/down_light.svg" if theme == "light" else "src/styling/theme_icons/down_dark.svg"
        
        return f"""
            /* ===== GLOBAL OVERRIDES ===== */
            
            QMainWindow {{
                /* QPalette handles background via Window role */
                border: none;
                margin: 0px;
                padding: 0px;
                outline: none;
            }}
            
            /* ===== SCROLLBARS ===== */
            /* QPalette doesn't style these well, so we use QSS */
            
            QScrollBar:vertical {{
                width: 12px;
                background: {colors["surface.secondary"]};
                border: none;
            }}
            
            QScrollBar::handle:vertical {{
                background: {colors["border.default"]};
                border-radius: 6px;
                min-height: 20px;
            }}
            
            QScrollBar::handle:vertical:hover {{
                background: {colors["action.hover"]};
            }}
            
            QScrollBar:horizontal {{
                height: 12px;
                background: {colors["surface.secondary"]};
                border: none;
            }}
            
            QScrollBar::handle:horizontal {{
                background: {colors["border.default"]};
                border-radius: 6px;
                min-width: 20px;
            }}
            
            QScrollBar::handle:horizontal:hover {{
                background: {colors["action.hover"]};
            }}
            
            QScrollBar::add-line, QScrollBar::sub-line {{
                border: none;
                background: none;
            }}
            
            /* ===== TOOLBARS ===== */
            
            QToolBar {{
                /*QToolBar in Qt is a complex container widget 
                that doesn't render borders or backgrounds 
                the way you'd expect.*/
            }}
            
            QToolBar::separator {{
                /*QToolBar in Qt is a complex container widget 
                that doesn't render borders or backgrounds 
                the way you'd expect.*/
            }}
            
            /* ===== MENUS ===== */
            
            QMenuBar {{
                spacing: 4px;
                padding: 2px;
                border-bottom: 1px solid {colors["border.subtle"]};
                /* Background/text from QPalette */
            }}
            
            QMenuBar::item {{
                padding: 4px 8px;
                border-radius: 3px;
            }}
            
            QMenuBar::item:selected {{
                background-color: {colors["action.primary"]};
                color: {colors["text.inverted"]};
            }}
            
            QMenu {{
                border: 1px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 4px;
                /* Background from QPalette.Base */
            }}
            
            QMenu::item {{
                padding: 6px 24px 6px 12px;
                border-radius: 3px;
            }}
            
            QMenu::item:selected {{
                background-color: {colors["action.primary"]};
                color: {colors["text.inverted"]};
            }}
            
            QMenu::separator {{
                height: 1px;
                background: {colors["border.subtle"]};
                margin: 4px 8px;
            }}
            
            /* ===== STATUSBAR ===== */
            
            QStatusBar {{
                background-color: {colors["surface.secondary"]};
                border: none;
                padding: 0px;
                margin: 0px;
            }}
            
            QStatusBar::item {{
                border: none;
                padding: 2px 8px;
                margin: 0px;
            }}

            
            /* ===== BUTTONS ===== */
            
            QPushButton {{
                border: 0px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 6px 8px;
                min-height: 14px;
                color: {colors["text.primary"]};
                /* Background from QPalette */
            }}
            
            QPushButton:hover {{
                background-color: {colors["action.hover"]};
                border-color: {colors["action.hover"]};
                color: {colors["text.inverted"]};
            }}
            
            QPushButton:pressed {{
                background: {colors["action.pressed"]};
                color: {colors["text.inverted"]};
                border-color: {colors["action.pressed"]};
            }}
            
            QPushButton:checked {{
                background-color: {colors["action.primary"]};
                color: {colors["text.inverted"]};
                border-color: {colors["border.focus"]};
            }}
            
            QPushButton:disabled {{
                /* Handled by QPalette.Disabled */
            }}
            
            /* ===== LIST WIDGETS ===== */
            
            QListWidget {{
                border: 0px solid red;
                outline: none;
                /* Background/selection from QPalette */
            }}
            
            QListWidget::item {{
                padding: 0px;
                border-radius: 0px;
                border: 0px solid blue;
            }}
            
            QListWidget::item:hover {{
                background-color: {colors["surface.tertiary"]};
            }}
            
            QListWidget::item:selected {{
                border: none;
                background-color: {colors["action.primary"]};
                color: {colors["text.inverted"]};
            }}
            
            /* ===== TEXT EDITORS ===== */
            
            QTextEdit, QPlainTextEdit {{
                border: 1px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 4px;
                /* Background/text from QPalette */
            }}
            
            QTextEdit:focus, QPlainTextEdit:focus {{
                border-color: {colors["border.focus"]};
            }}
            
            /* ===== LINE EDITS ===== */
            
            QLineEdit {{
                border: 1px solid {colors["border.default"]};

            }}
            
            QLineEdit:focus {{
                border-color: {colors["border.focus"]};
            }}
            
            /* ===== COMBO BOXES ===== */
            
            QComboBox {{
                border: 1px solid {colors["border.default"]};
                padding: 4px 8px 4px 8px; /* Adjust padding for better touch targets */
                /*min-width: 150px;*/
            }}
            
            QComboBox:hover {{
                border-color: {colors["action.hover"]};
            }}
            
            QComboBox QAbstractItemView {{
                background-color: {colors["surface.tertiary"]};
                border: 0px solid {colors["border.default"]};
                border-radius: 4px;
                selection-background-color: {colors["action.primary"]};
                selection-color: {colors["text.inverted"]};
                outline: none;
            }}

            QComboBox QAbstractItemView::item:selected {{
                background-color: {colors["action.primary"]};
                color: {colors["text.inverted"]};
            }}
            
            QComboBox QAbstractItemView::item {{
                padding: 4px 8px;
            }}
            
            QComboBox QAbstractItemView::item:hover {{
                background-color: {colors["action.hover"]};
                outline: none;
            }}
            
            /* ===== SPIN BOXES ===== */
            
            QSpinBox {{
                border: 1px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 4px;
                padding-right: 18px;
            }}
            
            QSpinBox:hover {{
                border-color: {colors["action.hover"]};
            }}
            
            QSpinBox::up-button {{
                subcontrol-origin: padding;
                subcontrol-position: top right;
                width: 16px;
                border-left: 1px solid {colors["border.default"]};
                background-color: {colors["surface.secondary"]};
            }}
            
            QSpinBox::down-button {{
                subcontrol-origin: padding;
                subcontrol-position: bottom right;
                width: 16px;
                border-left: 1px solid {colors["border.default"]};
                background-color: {colors["surface.tertiary"]};
            }}
            
            QSpinBox::up-button:hover, QSpinBox::down-button:hover {{
                background-color: {colors["surface.tertiary"]};
            }}
            
            QSpinBox::up-arrow {{
                image: url({up_icon});
                width: 8px;
                height: 8px;
            }}
            
            QSpinBox::down-arrow {{
                image: url({down_icon});
                width: 8px;
                height: 8px;
            }}
            
            /* ===== DOCK WIDGETS ===== */
            
            QDockWidget {{
                border: none;
                titlebar-close-icon: url(close.png);
                titlebar-normal-icon: url(float.png);
            }}
            
            QDockWidget::title {{
                padding: 6px;
                border-bottom: 1px solid {colors["border.subtle"]};
                text-align: left;
                /* Background/text from QPalette */
            }}
            
            /* ===== CUSTOM NOTEBOOK COMPONENTS (for future) ===== */
            
            /* These will be added as notebook components are built */
            
        """
