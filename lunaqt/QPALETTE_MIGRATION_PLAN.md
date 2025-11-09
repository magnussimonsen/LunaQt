# QPalette Migration Plan

## Overview

Migrate LunaQt's theme system from QSS-heavy approach to **QPalette + minimal QSS** architecture. This will:
- Work with Qt's style engine instead of fighting it
- Reduce maintenance burden
- Improve cross-platform consistency
- Better performance
- Easier to extend for notebook components

---

## Current State Analysis

### Existing Files to Refactor
```
lunaqt/src/themes/
├── colors.py              # ❌ Remove - Replace with semantic tokens
├── stylesheet.py          # ❌ Remove - Replace with minimal QSS
└── __init__.py            # ✅ Keep - Update exports

lunaqt/src/core/
└── theme_manager.py       # 🔄 Major refactor - Add QPalette logic

lunaqt/src/gui/style/
├── theme_updater.py       # 🔄 Simplify - Less manual updates needed
└── ...
```

### Current Approach (to be replaced)
1. Large `COLORS` dict with many specific color names
2. `get_stylesheet()` generates massive QSS strings
3. Manual `apply_theme_styles()` for widget-specific updates
4. Colors hardcoded throughout QSS

### New Approach
1. **Semantic color tokens** - Small, meaningful vocabulary
2. **QPalette** - Let Qt handle most coloring automatically
3. **Minimal QSS** - Only for layout/borders/specifics QPalette can't handle
4. **Centralized theme application** - Single source of truth

---

## Migration Steps

### Phase 1: Create New Theme Foundation (Day 1)

#### Step 1.1: Define Semantic Color System

**Create:** `lunaqt/src/themes/semantic_colors.py`

```python
"""Semantic color tokens for QPalette-based theming."""

from typing import Dict, Literal

ThemeMode = Literal["light", "dark"]


class SemanticColors:
    """
    Minimal, semantic color vocabulary.
    Each role maps to QPalette colors or custom tokens.
    """
    
    # Define semantic roles (not specific colors)
    TOKENS: Dict[ThemeMode, Dict[str, str]] = {
        "light": {
            # Surface colors (backgrounds)
            "surface.primary": "#FFFFFF",        # Main window background
            "surface.secondary": "#F5F5F5",      # Sidebar, menubar, panels
            "surface.tertiary": "#F0F0F0",       # Secondary toolbars
            "surface.elevated": "#FFFFFF",       # Dialogs, popups (same as primary for light)
            
            # Text colors
            "text.primary": "#000000",           # Primary text
            "text.secondary": "#666666",         # Muted text
            "text.disabled": "#AAAAAA",          # Disabled state
            "text.inverted": "#FFFFFF",          # Text on dark backgrounds
            
            # Interactive colors
            "action.primary": "#0078D4",         # Selections, focus, primary actions
            "action.hover": "#1A8CFF",           # Hover state
            "action.pressed": "#005A9E",         # Active/pressed state
            "action.disabled": "#DDDDDD",        # Disabled buttons
            
            # Border colors
            "border.default": "#CCCCCC",         # Standard borders
            "border.subtle": "#EEEEEE",          # Subtle dividers
            "border.focus": "#0078D4",           # Focus indicators
            
            # Status colors (for future use)
            "status.success": "#107C10",
            "status.warning": "#CA5010",
            "status.error": "#D13438",
            "status.info": "#0078D4",
            
            # Code/notebook specific
            "code.background": "#F8F8F8",
            "code.text": "#1E1E1E",
            "code.selection": "#ADD6FF",
        },
        
        "dark": {
            # Surface colors
            "surface.primary": "#121212",        # Main window background
            "surface.secondary": "#2D2D2D",      # Sidebar, menubar, panels
            "surface.tertiary": "#2D2D2D",       # Secondary toolbars
            "surface.elevated": "#3A3A3A",       # Dialogs, popups (elevated)
            
            # Text colors
            "text.primary": "#FFFFFF",
            "text.secondary": "#AAAAAA",
            "text.disabled": "#555555",
            "text.inverted": "#000000",
            
            # Interactive colors
            "action.primary": "#0E639C",         # Selections, focus
            "action.hover": "#1177BB",
            "action.pressed": "#094771",
            "action.disabled": "#4A4A4A",
            
            # Border colors
            "border.default": "#444444",
            "border.subtle": "#333333",
            "border.focus": "#0E639C",
            
            # Status colors
            "status.success": "#4EC9B0",
            "status.warning": "#CE9178",
            "status.error": "#F48771",
            "status.info": "#0E639C",
            
            # Code/notebook specific
            "code.background": "#1E1E1E",
            "code.text": "#D4D4D4",
            "code.selection": "#264F78",
        }
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
        return cls.TOKENS[theme][token]
    
    @classmethod
    def get_all(cls, theme: ThemeMode) -> Dict[str, str]:
        """Get all color tokens for a theme."""
        return cls.TOKENS[theme].copy()
```

#### Step 1.2: Create QPalette Builder

**Create:** `lunaqt/src/themes/palette_builder.py`

```python
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
```

#### Step 1.3: Create Minimal QSS Generator

**Create:** `lunaqt/src/themes/minimal_qss.py`

```python
"""Minimal QSS for things QPalette cannot handle."""

from .semantic_colors import SemanticColors, ThemeMode


class MinimalQSS:
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
        
        return f"""
            /* ===== GLOBAL OVERRIDES ===== */
            
            QMainWindow {{
                /* QPalette handles background via Window role */
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
                spacing: 4px;
                padding: 4px;
                border-bottom: 1px solid {colors["border.subtle"]};
                /* Background handled by QPalette.Window */
            }}
            
            QToolBar::separator {{
                width: 1px;
                background: {colors["border.subtle"]};
                margin: 4px 8px;
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
                /* Hover handled by QPalette.Highlight */
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
                /* Selection from QPalette.Highlight */
            }}
            
            QMenu::separator {{
                height: 1px;
                background: {colors["border.subtle"]};
                margin: 4px 8px;
            }}
            
            /* ===== BUTTONS ===== */
            
            QPushButton {{
                border: 1px solid {colors["border.default"]};
                border-radius: 4px;
                padding: 6px 16px;
                min-height: 24px;
                /* Background/text from QPalette */
            }}
            
            QPushButton:hover {{
                border-color: {colors["action.hover"]};
            }}
            
            QPushButton:pressed {{
                background: {colors["action.pressed"]};
                border-color: {colors["action.pressed"]};
            }}
            
            QPushButton:disabled {{
                /* Handled by QPalette.Disabled */
            }}
            
            /* ===== LIST WIDGETS ===== */
            
            QListWidget {{
                border: none;
                outline: none;
                /* Background/selection from QPalette */
            }}
            
            QListWidget::item {{
                padding: 4px;
                border-radius: 3px;
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
                border-radius: 4px;
                padding: 6px;
                /* Background/text from QPalette */
            }}
            
            QLineEdit:focus {{
                border-color: {colors["border.focus"]};
            }}
            
            /* ===== CUSTOM NOTEBOOK COMPONENTS (for future) ===== */
            
            /* These will be added as notebook components are built */
            
        """
```

---

### Phase 2: Refactor Theme Manager (Day 1-2)

#### Step 2.1: Rewrite ThemeManager

**Modify:** `lunaqt/src/core/theme_manager.py`

```python
"""Theme management with QPalette-first approach."""

from typing import Optional
from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import Signal, QObject

from ..constants.types import ThemeMode
from ..themes.palette_builder import PaletteBuilder
from ..themes.minimal_qss import MinimalQSS
from ..themes.semantic_colors import SemanticColors


class ThemeManager(QObject):
    """
    Manages application theme using QPalette + minimal QSS.
    
    QPalette handles most colors automatically.
    QSS only for structural styling (borders, spacing, etc).
    """
    
    # Signal emitted when theme changes
    theme_changed = Signal(str)  # emits theme name ("light" or "dark")
    
    def __init__(self, initial_theme: ThemeMode = "light") -> None:
        """Initialize theme manager.
        
        Args:
            initial_theme: Starting theme mode
        """
        super().__init__()
        self._current_theme: ThemeMode = initial_theme
        self._window: Optional[QMainWindow] = None
    
    def set_window(self, window: QMainWindow) -> None:
        """Set the main window to apply themes to.
        
        Args:
            window: Main window instance
        """
        self._window = window
    
    @property
    def current_theme(self) -> ThemeMode:
        """Get current theme mode."""
        return self._current_theme
    
    def get_color(self, token: str) -> str:
        """
        Get semantic color for current theme.
        
        Args:
            token: Semantic token like 'surface.primary'
            
        Returns:
            Hex color string
        """
        return SemanticColors.get(self._current_theme, token)
    
    def apply_theme(self, theme: ThemeMode) -> None:
        """
        Apply theme to the application.
        
        Uses QPalette for colors, minimal QSS for structure.
        
        Args:
            theme: Theme mode to apply
        """
        if not self._window:
            raise RuntimeError("Window not set. Call set_window() first.")
        
        self._current_theme = theme
        
        # 1. Apply QPalette (handles most colors automatically)
        palette = PaletteBuilder.build(theme)
        QApplication.instance().setPalette(palette)
        
        # 2. Apply minimal QSS (only for structure/borders)
        qss = MinimalQSS.get(theme)
        QApplication.instance().setStyleSheet(qss)
        
        # 3. Emit signal for custom components
        self.theme_changed.emit(theme)
        
        # Force repaint
        self._window.update()
    
    def toggle_theme(self) -> None:
        """Toggle between light and dark theme."""
        new_theme: ThemeMode = "dark" if self._current_theme == "light" else "light"
        self.apply_theme(new_theme)
    
    def set_light_theme(self) -> None:
        """Switch to light theme."""
        self.apply_theme("light")
    
    def set_dark_theme(self) -> None:
        """Switch to dark theme."""
        self.apply_theme("dark")
```

---

### Phase 3: Update Notebook Components (Day 2)

#### Step 3.1: Create Notebook-Specific Styles

**Create:** `lunaqt/src/gui/notebook/styles/notebook_qss.py`

```python
"""Notebook-specific QSS additions."""

from ....themes.semantic_colors import SemanticColors, ThemeMode


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
                font-family: 'Fira Code', 'Consolas', monospace;
            }}
            
            /* ===== MARKDOWN CELLS ===== */
            
            MarkdownCell QTextEdit {{
                border: none;
                /* Background/text from QPalette */
            }}
            
            /* ===== CELL TOOLBARS ===== */
            
            BaseToolbar {{
                border-bottom: 1px solid {colors["border.subtle"]};
                spacing: 4px;
                padding: 4px 8px;
            }}
        """
```

---

### Phase 4: Clean Up Old Files (Day 2)

#### Files to Delete:
- ❌ `lunaqt/src/themes/stylesheet.py` (replaced by minimal_qss.py)
- ❌ `lunaqt/src/gui/style/theme_updater.py` (no longer needed with QPalette)

#### Files to Update:
- 🔄 `lunaqt/src/themes/__init__.py` - Update exports
- 🔄 `lunaqt/src/themes/colors.py` - Delete and replace with semantic_colors.py

#### Step 4.1: Update Theme Module Exports

**Replace:** `lunaqt/src/themes/__init__.py`

```python
"""Theme system with QPalette-first approach."""

from .semantic_colors import SemanticColors, ThemeMode
from .palette_builder import PaletteBuilder
from .minimal_qss import MinimalQSS

__all__ = [
    "SemanticColors",
    "ThemeMode",
    "PaletteBuilder",
    "MinimalQSS",
]
```

---

### Phase 5: Update Main Window Integration (Day 2-3)

#### Step 5.1: Update main_window.py

**Modify:** `lunaqt/src/gui/main_window.py`

Remove old theme update calls, connect to new theme_changed signal:

```python
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        
        # Theme manager
        self.theme_manager = ThemeManager()
        self.theme_manager.set_window(self)
        
        # Connect to theme changes for custom updates
        self.theme_manager.theme_changed.connect(self.on_theme_changed)
        
        # Apply initial theme
        self.theme_manager.apply_theme("light")
        
    def on_theme_changed(self, theme: str):
        """Handle theme changes for custom components."""
        # Most widgets update automatically via QPalette
        # Only handle special cases here (if any)
        pass
```

---

## Migration Checklist

### Day 1: Foundation
- [ ] Create `semantic_colors.py` with token system
- [ ] Create `palette_builder.py` for QPalette construction
- [ ] Create `minimal_qss.py` for structural styles
- [ ] Test basic theme switching

### Day 2: Integration
- [ ] Refactor `theme_manager.py` to use QPalette
- [ ] Update `themes/__init__.py` exports
- [ ] Delete old `colors.py` and `stylesheet.py`
- [ ] Create `notebook_qss.py` for future notebook styles
- [ ] Update `main_window.py` integration

### Day 3: Testing & Polish
- [ ] Test all existing UI components with new theme system
- [ ] Fix any visual regressions
- [ ] Verify theme switching works smoothly
- [ ] Update any hardcoded color references
- [ ] Document new theme system usage

### Day 4: Documentation
- [ ] Update README with new theme architecture
- [ ] Add developer guide for adding new themed components
- [ ] Document semantic token naming conventions

---

## Benefits After Migration

✅ **Simpler codebase**: ~80% less styling code  
✅ **Better performance**: QPalette is faster than QSS parsing  
✅ **Cross-platform**: Works with native Qt behavior  
✅ **Maintainable**: Semantic tokens are self-documenting  
✅ **Extensible**: Easy to add notebook-specific styles  
✅ **Future-proof**: Clean foundation for advanced features  

---

## Migration Strategy

1. **Create new files alongside old** (don't delete yet)
2. **Test new system in isolation**
3. **Switch ThemeManager to new system**
4. **Verify all components still work**
5. **Delete old files only when confident**

This allows rollback if issues arise.

---

## Next Steps After Migration

Once QPalette system is working:

1. Build notebook components using semantic tokens
2. Add notebook-specific QSS as needed (minimal)
3. Components automatically adapt to theme changes
4. Easy to add new themes (just define new token set)

---

## Questions?

- Semantic tokens unclear? → Reference SemanticColors.TOKENS
- QPalette role confused? → Check PaletteBuilder mappings
- Need custom widget colors? → Use theme_manager.get_color(token)
- QSS override needed? → Add to NotebookQSS, not MinimalQSS
