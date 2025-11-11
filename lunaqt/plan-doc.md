# LunaQt Notebook Architecture Plan

## Overview

LunaQt will implement a Jupyter-like notebook system using PySide6 (Qt6) with a focus on maintainability, theme integration, and clean separation of concerns.

This document follows an MVP-first approach: build the smallest useful prototype with a clean, extensible structure, then layer in professional features. Non-essential items (accessibility, risk register, advanced execution, etc.) are deferred but scaffolded in the repository for smooth expansion later.

## MVP-first plan (keep it small, ready to grow)

MVP goals (functional, minimal):
- App starts with themed UI (QPalette-first + minimal QSS)
- MainWindow with a resizable settings side panel and a central placeholder Notebook area
- Theme switching (light/dark) via ThemeManager
- Basic settings controls (comboboxes/spinboxes) wired and styled
- Project structure in place for data managers and notebook UI, even if not fully implemented

Out-of-scope for MVP (scaffold only):
- Notebook persistence (JSON), cell execution, markdown rendering
- Command palette, undo/redo, complex outputs
- Accessibility audits, risk register, full test matrix

Acceptance criteria:
- The app launches without errors on Windows
- Theme switching works and repaints widgets
- Settings panel can be shown/hidden and resized without snapping glitches
- UI uses no hard-coded colors; QSS overrides are structural only
- Repository contains future-ready folders with short README placeholders

## Progress checklist

Use this checklist to track MVP progress and planned build-outs. Update in PRs as items are completed.

### Theme and styling
- [x] Semantic color tokens in `themes/semantic_colors.py`
- [x] `PaletteBuilder` constructs `QPalette` with roles
- [x] Minimal QSS in `themes/minimal_qss.py` applied app-wide
- [x] `ThemeManager` applies palette and QSS, emits change signal
- [x] QSpinBox arrows via SVG (up/down × light/dark), offline-safe

### UI shell
- [x] `MainWindow` with settings side panel; resizable
- [x] Settings controls wired (QComboBox with AdjustToContents, QSpinBox)
- [x] Persist window geometry/state via QSettings
- [x] Document notebook cell GUI plan (this file)
- [ ] Consider switching settings panel to QSplitter if resize quirks persist

### Widgets polish
- [x] QComboBox dropdown background and selection colors
- [x] Hover states for buttons, menus, list widgets
- [x] QSpinBox button backgrounds and icon rendering
- [ ] QDockWidget title styling (custom title bar if needed)

### Structure for growth
- [x] Scaffolding created: `src/models/`, `src/utils/`, `src/gui/notebook/cells/`, `src/gui/notebook/toolbars/`
- [x] Core stubs added: `core/data_store.py`, `core/cell_manager.py`, `core/notebook_manager.py`
- [x] READMEs in new folders explaining deferred implementation
- [x] UUID helper in `utils` for IDs
- [x] `NotebookView` container (QListWidget-based) placeholder

### Data and persistence
- [x] DataStore MVP (JSON read/write + atomic save)
- [x] NotebookManager API (create/open/save)
- [x] Add `schema_version` to models
- [x] Use per-OS data directory (QStandardPaths/AppDirs)
- [x] CellManager API (create/update/delete/get)
- [x] Cell widgets (BaseCell, CodeCell, MarkdownCell)
- [x] Wire NotebookView into MainWindow central area
- [x] Implement NotebookView behavior (selection, insert/delete/move, placeholder)

### Commands and shortcuts (post-MVP)
- [ ] Centralized command registry (IDs, labels, shortcuts)
- [ ] Wire menubar/shortcuts to registry
- [ ] Command palette shell consuming registry

### Packaging and resources (post-MVP)
- [ ] Move icons/QSS to Qt Resource System (qrc)
- [ ] Ensure high-DPI-safe icon scaling

### Tests and tooling (post-MVP)
- [ ] Minimal unit tests for DataStore and NotebookManager
- [ ] Enable mypy, ruff, black in CI

---

## Data Structure Architecture

### Core Principles

1. **Unique IDs**: Every cell and notebook has a unique identifier (UUID)
2. **JSON Storage**: All cell data persisted in JSON format
3. **ID-based References**: Notebooks store ordered lists of cell IDs (not cell data)
4. **Separation**: Data layer completely independent of UI layer

### Data Models (scaffold for later)

#### Cell Model (`models/cell.py`) – planned, not required for MVP

```python
{
    "cell_id": "uuid-string",
    "cell_type": "code" | "markdown" | "raw",
    "content": "cell content as string",
    "metadata": {
        "execution_count": int,
        "collapsed": bool,
        "language": "python" | "markdown",
        "tags": []
    },
    "outputs": [
        {
            "output_type": "stream" | "execute_result" | "error",
            "data": {...},
            "execution_count": int
        }
    ],
    "created_at": "ISO-8601 timestamp",
    "modified_at": "ISO-8601 timestamp"
}
```

**Cell Types:**
- `code`: Executable code (Python, initially)
- `markdown`: Formatted text with Markdown and KaTeX in inline between $...$ and KaTeX block between $$ ... $$
- `raw`: Plain text, no execution or rendering

#### Notebook Model (`models/notebook.py`) – planned, not required for MVP

```python
{
    "notebook_id": "uuid-string",
    "title": "Notebook Title",
    "cell_ids": [
        "cell-uuid-1",
        "cell-uuid-2",
        "cell-uuid-3"
    ],
    "metadata": {
        "kernel": "python3",
        "language": "python",
        "author": "",
        "tags": []
    },
    "created_at": "ISO-8601 timestamp",
    "modified_at": "ISO-8601 timestamp"
}
```

**Key Feature (future)**: Reordering cells only modifies the `cell_ids` array order—no cell data duplication.

---

## Storage Architecture (post-MVP)

### File Structure

Use OS-appropriate data locations via QStandardPaths/AppDirs (not hard-coded ~/.lunaqt). Example structure (conceptual):

```
<AppData>/LunaQt/
├── notebooks/
│   ├── notebook-uuid-1.json
│   ├── notebook-uuid-2.json
│   └── ...
└── cells/
    ├── cell-uuid-1.json
    ├── cell-uuid-2.json
    └── ...
```

### Data Store (`core/data_store.py`)

**Responsibilities (planned):**
- Load/save individual cells from JSON
- Load/save notebooks from JSON
- Handle file I/O and error handling
- Provide atomic write operations
- Cache recently accessed cells

**Key Methods (planned):**
```python
load_cell(cell_id: str) -> dict
save_cell(cell_data: dict) -> bool
load_notebook(notebook_id: str) -> dict
save_notebook(notebook_data: dict) -> bool
delete_cell(cell_id: str) -> bool
delete_notebook(notebook_id: str) -> bool
list_notebooks() -> list[dict]
```

---

## Manager Layer (scaffold now, implement later)

### Cell Manager (`core/cell_manager.py`) – stubbed

**Responsibilities:**
- Create new cells with unique IDs
- Load cell data from storage
- Update cell content
- Delete cells (with safety checks)
- Manage cell metadata
- Handle cell type conversions

**Key Methods:**
```python
create_cell(cell_type: str, content: str = "") -> str  # Returns cell_id
get_cell(cell_id: str) -> Cell
update_cell(cell_id: str, updates: dict) -> bool
delete_cell(cell_id: str) -> bool
convert_cell_type(cell_id: str, new_type: str) -> bool
```

### Notebook Manager (`core/notebook_manager.py`) – stubbed

**Responsibilities:**
- Create/open/close notebooks
- Manage cell order within notebooks
- Add/remove cells from notebooks
- Save notebook state
- Handle "active notebook" concept

**Key Methods:**
```python
create_notebook(title: str) -> str  # Returns notebook_id
open_notebook(notebook_id: str) -> Notebook
close_notebook(notebook_id: str, save: bool = True) -> bool
add_cell(notebook_id: str, cell_id: str, position: int = -1) -> bool
remove_cell(notebook_id: str, cell_id: str) -> bool
move_cell(notebook_id: str, cell_id: str, new_position: int) -> bool
get_cell_order(notebook_id: str) -> list[str]
save_notebook(notebook_id: str) -> bool
```

### Cell Executor (`core/cell_executor.py`) – future

**Responsibilities (Future):**
- Execute code cells
- Capture output (stdout, stderr, results)
- Manage kernel/interpreter
- Handle execution state
- Interrupt/restart execution

---

## UI Architecture (MVP + growth path)

### Component Hierarchy

```
MainWindow (QMainWindow)
├── MenuBar
├── ToolbarStack (QStackedWidget)
│   ├── DefaultToolbar (no cell selected)
│   ├── CodeToolbar (code cell selected)
│   └── MarkdownToolbar (markdown cell selected)
├── CommandPalette (QDialog)          # Ctrl+Shift+P
└── NotebookView (QWidget)
    └── QListWidget (with setItemWidget)
        ├── QListWidgetItem → CodeCell
        ├── QListWidgetItem → MarkdownCell
        └── QListWidgetItem → CodeCell
```

**Architecture Decision: Hybrid Widget Approach (MVP)**

We're using `QListWidget` with `setItemWidget()` instead of pure `QVBoxLayout` or full `QAbstractListModel`:

**Advantages:**
- Semi-virtualized scrolling (better than plain layout)
- Simpler than full Model/View/Delegate pattern
- Built-in selection, keyboard navigation, drag-drop
- Easy to implement initially
- Can migrate to `QListView` + delegates later if needed (100+ cells)

**Future Migration Path:**
If performance becomes an issue with very large notebooks (100+ cells), we can refactor to:
- `QAbstractListModel` for data
- `QListView` for display
- `QStyledItemDelegate` for custom painting
- Full virtualization (only visible cells rendered)

### Base Components (planned)

#### BaseCell (`gui/notebook/cells/base_cell.py`)

**Template for all cell types**

**Features:**
- Selection state management
- Visual selection indicator
- Mouse click handling
- Signal emissions (selected, deleted, moved)
- Common layout structure
- Theme-aware styling
- Data serialization interface

**Signals:**
```python
selected = Signal(str, str)  # cell_id, cell_type
deleted = Signal(str)        # cell_id
moved = Signal(str, int)     # cell_id, direction
content_changed = Signal(str, str)  # cell_id, new_content
```

**Properties:**
```python
cell_id: str
cell_type: str
is_selected: bool
```

#### BaseToolbar (`gui/notebook/toolbars/base_toolbar.py`)

**Template for all context-sensitive toolbars**

**Features:**
- Consistent layout (QHBoxLayout)
- Theme-aware styling
- Enable/disable all actions
- Common button spacing

**Override Method:**
```python
setupUI()  # Add toolbar-specific buttons
```

### Cell Types (planned)

#### CodeCell (`gui/notebook/cells/code_cell.py`)

**Inherits:** `BaseCell`

**Components:**
- Code editor (QTextEdit with syntax highlighting)
- Execution count indicator
- Output display area (collapsible)
- Cell toolbar (run, delete, move)

**Features:**
- Syntax highlighting (future: use QScintilla or similar)
- Line numbers
- Auto-indentation
- Bracket matching
- Code completion (future)

#### MarkdownCell (`gui/notebook/cells/markdown_cell.py`)

**Inherits:** `BaseCell`

**Components:**
- Editor mode: QTextEdit (plain text)
- Render mode: QTextBrowser (rendered HTML)
- Toggle button (edit/preview)

**Features:**
- Markdown rendering (using `markdown` library)
- Live preview toggle
- LaTeX support (future)
- Image embedding

### Toolbar Types (planned)

#### DefaultToolbar (`gui/notebook/toolbars/default_toolbar.py`)

**Shown when:** No cell selected

**Actions:**
- Add Cell (dropdown: Code, Markdown, Raw)
- Save Notebook
- Export (future)
- Kernel controls (future)

#### CodeToolbar (`gui/notebook/toolbars/code_toolbar.py`)

**Shown when:** Code cell selected

**Actions:**
- Run Cell (▶)
- Run All Above
- Run All Below
- Clear Output
- Interrupt Kernel (⏹)
- Cell type dropdown

#### MarkdownToolbar (`gui/notebook/toolbars/markdown_toolbar.py`)

**Shown when:** Markdown cell selected

**Actions:**
- Bold (B)
- Italic (I)
- Heading (H1, H2, H3)
- Link (🔗)
- Image (🖼)
- Code block
- Cell type dropdown

---

## Notebook Cell GUI Plan (MVP)

This section defines the concrete interaction contract for NotebookView and cell widgets for the MVP. It covers selection rules, insert/delete/move behavior, placeholder handling, and how actions enable/disable based on state.

### Goals and constraints

- Single selection only (exactly 0 or 1 selected cell at a time)
- Keyboard and mouse friendly; drag-and-drop is optional later
- Operations: insert (above/below/end), delete, move up/down, convert type (optional)
- Persist via NotebookManager/CellManager; UI reflects persisted order and selection

### Internal structures (NotebookView)

- Underlying: QListWidget with setItemWidget(CodeCell/MarkdownCell)
- Registry mappings:
    - `cell_id -> (QListWidgetItem, BaseCell)`
    - `QListWidgetItem -> cell_id`
- Public helpers:
    - `get_selected_index() -> int | None`
    - `get_selected_cell_id() -> str | None`
    - `select_index(i: int) -> None`
    - `select_cell(cell_id: str) -> None`
    - `add_cell(cell_id: str, cell_type: str, content: str, position: int | None = None) -> None`
    - `remove_cell(cell_id: str) -> None`
    - `move_cell(cell_id: str, delta: int) -> None`  # delta = -1 (up) or +1 (down)

### Selection behavior

- Click on a cell selects it and deselects any previous selection
- Selection is visually indicated by BaseCell (e.g., border highlight)
- When selection changes, NotebookView emits `selection_changed(cell_id: str, cell_type: str, index: int)`

### Operations and rules

1) Insert
- Insert Above: if a cell is selected at index i, insert new cell at i; select the new cell
- Insert Below: if selected at index i, insert at i+1; select the new cell
- Insert At End: append after the last cell; select the new cell
- If no selection:
    - For Above/Below: treat as End
    - For End: append
- Persistence: create via CellManager.create_cell(type), add to notebook via NotebookManager.add_cell(notebook_id, cell_id, position), then add to UI

2) Delete
- Requires a selected cell; if none, no-op
- Remove from NotebookManager, then remove from UI registry and list
- Selection fallback after delete:
    - If an item now exists at the same index, select it
    - Else, if there is a previous item (index-1), select it
    - Else, no selection; show placeholder

3) Move Up / Move Down
- With selected index i:
    - Move Up only if i > 0 (delta = -1)
    - Move Down only if i < count-1 (delta = +1)
- Call NotebookManager.move_cell(notebook_id, cell_id, new_position)
- Update UI order (takeItem/insertItem or swap); keep the same cell selected post-move

### Edge cases and enable/disable logic

- 0 cells: show placeholder; only “Add” actions enabled; Delete/Move disabled
- 1 cell: Delete enabled when selected; Move Up/Down disabled
- At first cell: Move Up disabled
- At last cell: Move Down disabled
- No selection but non-empty notebook: Add actions enabled; Delete/Move disabled

NotebookView emits a compact state signal after any change:
- `state_changed(can_insert: bool, can_delete: bool, can_move_up: bool, can_move_down: bool, count: int, has_selection: bool)`

Toolbars subscribe to `selection_changed` and `state_changed` to enable/disable their buttons.

### Placeholder behavior (empty notebook)

- Show a centered, low-contrast panel with text: “No cells yet. Use the Insert menu to add a cell.”
- No inline add buttons; cell insertion is driven by the menubar Insert actions
- When the first cell is added, hide the placeholder and select the new cell

### Load/open flow

1. MainWindow determines `notebook_id` and calls NotebookManager.open_notebook()
2. NotebookView.clear(), hide placeholder
3. For each `cell_id` in `NotebookManager.get_cell_order()`:
     - Load via CellManager.get_cell(cell_id)
     - Create appropriate cell widget and add to list/registry
4. If any cells were added, select index 0; else, show placeholder
5. Emit initial `state_changed` reflecting current count/selection

### Minimal signals (between layers)

- BaseCell emits:
    - `selected(cell_id: str, cell_type: str)`
    - `content_changed(cell_id: str, new_content: str)`
- NotebookView emits:
    - `selection_changed(cell_id: str, cell_type: str, index: int)`
    - `state_changed(can_insert: bool, can_delete: bool, can_move_up: bool, can_move_down: bool, count: int, has_selection: bool)`
    - `request_persist_reorder(new_order: list[str])` (optional if persistence is invoked directly)

### Tiny contract (I/O expectations)

- Inputs: current notebook_id, managers (NotebookManager, CellManager)
- Outputs: UI list of cells in manager order; signals for selection/state; operations delegate to managers then reflect in UI
- Error modes: failed persistence (show non-blocking toast/status), invalid indices are guarded and no-op
- Success: list and managers remain in sync; selection is always valid or empty; actions reflect current state

### Next steps to implement

- Implement NotebookView registry and helpers listed above
- Wire BaseCell click to NotebookView.selection handling
- Add toolbar buttons (Add Above/Below/End, Delete, Up, Down) and connect signals
- Implement enable/disable updates based on `state_changed`
- Cover edge cases with simple unit tests around ordering helpers (post-MVP optional)

## Styling System

**Architecture Decision: QPalette + Minimal QSS** ⭐

We use a **QPalette-first approach** with minimal QSS overrides. This follows Qt best practices and provides:
- Better performance (QPalette is faster than parsing QSS)
- Cross-platform consistency
- Automatic state handling (hover, focus, disabled)
- Less code to maintain

### Theme Architecture (aligns with best practices)

#### Semantic Color Tokens (`themes/semantic_colors.py`)

Minimal, semantically-named color vocabulary:

```python
TOKENS = {
    "light": {
        # Surfaces
        "surface.primary": "#FFFFFF",      # Main background
        "surface.secondary": "#F5F5F5",    # Panels, sidebar
        "surface.elevated": "#FFFFFF",     # Dialogs, popups
        
        # Text
        "text.primary": "#000000",
        "text.secondary": "#666666",
        "text.disabled": "#AAAAAA",
        
        # Actions
        "action.primary": "#0078D4",       # Selection, focus
        "action.hover": "#1A8CFF",
        "action.pressed": "#005A9E",
        
        # Borders
        "border.default": "#CCCCCC",
        "border.focus": "#0078D4",
        
        # Code-specific
        "code.background": "#F8F8F8",
        "code.text": "#1E1E1E",
    },
    "dark": { ... }
}
```

**Key Principle:** Use semantic names (`surface.primary`, `action.hover`) not specific names (`sidebar_bg`, `button_hover_blue`).

#### QPalette Builder (`themes/palette_builder.py`)

Maps semantic tokens to QPalette color roles:

```python
palette.setColor(QPalette.Window, colors["surface.primary"])
palette.setColor(QPalette.Text, colors["text.primary"])
palette.setColor(QPalette.Highlight, colors["action.primary"])
# ... etc
```

Qt automatically applies these to all widgets. Child widgets inherit properly.

#### Minimal QSS (`themes/minimal_qss.py`)

**Only** for things QPalette cannot handle:
- Borders, border-radius, shadows
- Spacing, padding, margins
- Scrollbar styling
- Layout-specific overrides

```python
QScrollBar:vertical {
    width: 12px;
    background: {surface.secondary};
}

QPushButton {
    border: 1px solid {border.default};
    border-radius: 4px;
    padding: 6px 16px;
    /* Background/text from QPalette automatically */
}
```

**What QPalette Handles Automatically:**
- ✅ Background colors (`QPalette.Window`, `QPalette.Base`)
- ✅ Text colors (`QPalette.Text`, `QPalette.WindowText`)
- ✅ Button colors (`QPalette.Button`, `QPalette.ButtonText`)
- ✅ Selection/highlight (`QPalette.Highlight`)
- ✅ Disabled states (`QPalette.Disabled`)
- ✅ Links (`QPalette.Link`)

**What Requires QSS:**
- ❌ Borders and border-radius
- ❌ Padding and margins
- ❌ Scrollbar custom styling
- ❌ Complex selectors (`:hover` on custom widgets)

### Notebook-Specific Styles (`gui/notebook/styles/notebook_qss.py`) – planned

Additional QSS for notebook components:

```python
BaseCell {
    border: 1px solid {border.default};
    border-radius: 4px;
    /* Background/text from QPalette */
}

BaseCell[selected="true"] {
    border: 2px solid {action.primary};
}

CodeCell QTextEdit {
    background-color: {code.background};  # Override for code
    font-family: 'Fira Code', monospace;
}
```

### Theme Application Flow

```
User changes theme
    ↓
ThemeManager.apply_theme(theme)
    ↓
1. Build QPalette from semantic tokens
    ↓
2. QApplication.setPalette(palette)  ← Affects ALL widgets automatically
    ↓
3. Apply minimal QSS for structure only
    ↓
4. Emit theme_changed signal
    ↓
5. Custom components update if needed
    ↓
All widgets repaint with new colors
```

### Adding New Themed Components

When creating new widgets:

1. **Use QPalette colors by default** - No explicit styling needed
2. **Add borders/spacing via QSS** if needed
3. **Use semantic tokens** via `theme_manager.get_color(token)`
4. **Avoid hardcoded colors** in Python code

Example:
```python
class MyWidget(QWidget):
    def __init__(self):
        # Background automatically from QPalette.Window
        # Text automatically from QPalette.Text
        
        # Only add QSS if need borders:
        self.setStyleSheet("""
            MyWidget {
                border: 1px solid palette(mid);
                border-radius: 4px;
            }
        """)
```

### Migration Note

See `QPALETTE_MIGRATION_PLAN.md` for full details on the QPalette migration from the old QSS-heavy system.

---

## Project Structure (MVP-ready scaffold)

```
lunaqt/src/
├── constants/
│   └── notebook_constants.py          # Cell types, defaults, configs (future)
│
├── models/                            # CREATED (placeholder)
│   ├── __init__.py                    # Stubs; README explains deferment
│   └── README.md
│
├── core/
│   ├── data_store.py                  # Stubbed for post-MVP JSON persistence
│   ├── cell_manager.py                # Stubbed manager
│   ├── notebook_manager.py            # Stubbed manager
│   └── theme_manager.py               # QPalette-based theme manager
│
├── themes/
│   ├── __init__.py
│   ├── semantic_colors.py             # Semantic color tokens
│   ├── palette_builder.py             # QPalette construction
│   └── minimal_qss.py                 # Minimal structural QSS
│
├── gui/
│   ├── main_window.py                 # Main application window
│   └── notebook/
│       ├── README.md                  # Planned components (placeholder)
│       ├── cells/                     # CREATED (placeholder)
│       │   └── __init__.py
│       ├── toolbars/                  # CREATED (placeholder)
│       │   └── __init__.py
│       └── styles/
│           ├── __init__.py
│           └── notebook_qss.py        # Notebook QSS (future)
│
└── utils/                             # CREATED (placeholder)
    └── __init__.py                    # Future helpers (UUID, etc.)
```

---

## Enhancements from Community Review (trimmed for MVP, queued for later)

### Implemented Design Decisions (MVP-aligned)

#### 1. Command Palette (Point 9)
**Status:** ✅ Approved and planned for Phase 4

A searchable command palette (Ctrl+Shift+P) will provide:
- Quick access to all application commands
- Fuzzy search for discoverability
- Keyboard-first workflow
- Professional UX similar to VS Code/Jupyter

**Implementation:**
- `gui/command_palette.py` - Dialog with search and command list
- Command registry pattern for extensibility
- Keybindings integrated with existing shortcuts

#### 2. List-Based Cell Rendering (Point 4)
**Status:** ✅ Hybrid approach adopted

Instead of pure `QVBoxLayout` or full Model/View, we're using **QListWidget with setItemWidget()**:

**Why this approach:**
- ✅ Semi-virtualized scrolling (better performance than plain layout)
- ✅ Simpler than full `QAbstractListModel` + delegates
- ✅ Built-in selection, keyboard nav, drag-drop support
- ✅ Can upgrade to full Model/View later if needed (100+ cells)

**Trade-offs considered:**
- Pure QVBoxLayout: Too simple, no virtualization, doesn't scale
- Full Model/View: Too complex initially, harder to debug interactive editors
- **QListWidget hybrid: Best balance for MVP** ⭐

**Future optimization path:**
If notebooks grow to 100+ cells and performance degrades:
1. Migrate to `QListView` + `QAbstractListModel`
2. Implement `QStyledItemDelegate` for custom cell painting
3. Enable full virtualization (only visible cells in memory)

### Under Consideration (post-MVP)

#### 3. Schema Versioning (Point 1)
**Priority:** High for Phase 1

Add to Cell and Notebook JSON:
```python
{
    "schema_version": 1,  # Enable future migrations
    "dirty": false,       # Track unsaved changes
    ...
}
```

**Benefits:**
- Safe format evolution
- Migration scripts for upgrades
- Crash recovery support

#### 4. Undo/Redo System (Point 3)
**Priority:** Medium for Phase 4-5

Use `QUndoStack` with command pattern:
- `AddCellCommand`, `DeleteCellCommand`, `MoveCellCommand`, `EditCellCommand`
- Native Ctrl+Z/Ctrl+Y support
- Better than manual state tracking

#### 5. Transaction Safety (Point 2)
**Priority:** Medium for Phase 1

Implement in `DataStore`:
```python
with data_store.transaction():
    # Notebook + cell updates
    # Atomic: all succeed or all fail
```

**Benefits:**
- Prevents partial writes on crash
- Ensures notebook ↔ cells consistency
- Use temp files + atomic rename

#### 6. Jupyter Compatibility (Point 5)
**Priority:** Very Low (Phase 6+)

Partial .ipynb support:
- Import/export basic notebooks
- Map cell types (code/markdown)
- Convert outputs to MIME bundles
- **Not required for MVP**

#### 7. Process-Isolated Execution (Point 6)
**Priority:** Low (Phase 6+)

When implementing `CellExecutor`:
- Use `jupyter_client` + ZMQ for kernel communication
- Separate process for Python execution (avoid GIL blocking UI)
- Enables kernel interrupts and restarts
- Multi-language support becomes easier

#### 8. Large Output Storage (Point 10)
**Priority:** Low (Phase 3-5)

Store outputs separately:
```
~/.lunaqt/
├── notebooks/
├── cells/
└── assets/
    └── <cell_id>/
        ├── plot.png
        └── data.csv
```

Reference in cell JSON:
```python
"outputs": [{
    "output_type": "display_data",
    "data": {
        "image/png": "file://assets/<cell_id>/plot.png"
    }
}]
```

#### 9. Additional ChatGPT Suggestions

**Accepted for implementation:**
- Multi-cell selection (Shift+Click, Ctrl+Click)
- Jupyter-style keyboard shortcuts (A/B for insert, DD for delete, M/Y for convert)
- Inline status indicators (execution time, last run)
- Centralized logging with rotating file handler

**Deferred to future phases:**
- Collaboration/CRDT support
- Git integration
- Extension API
- Cloud sync

---

## Key Design Patterns (kept simple for MVP)

### 1. Template Pattern
- `BaseCell` and `BaseToolbar` provide templates
- Subclasses override specific methods
- Ensures consistency across implementations

### 2. Observer Pattern
- Cells emit signals on state changes
- `NotebookView` observes cells
- `MainWindow` observes `NotebookView`
- Theme changes propagate through observers

### 3. Strategy Pattern
- Different toolbar strategies for different cell types
- `QStackedWidget` swaps active strategy
- No conditional UI logic in main window

### 4. Separation of Concerns
- **Models**: Pure data, no UI knowledge
- **Managers**: Business logic, coordinates models
- **UI**: Presentation only, delegates to managers
- **Styles**: Centralized, theme-integrated

---

## Data Flow Examples (future behavior)

### Creating a New Cell

```
User clicks "Add Cell" button
    ↓
DefaultToolbar emits add_cell_requested(cell_type="code")
    ↓
MainWindow receives signal
    ↓
MainWindow calls NotebookManager.add_cell(notebook_id, cell_type)
    ↓
NotebookManager calls CellManager.create_cell(cell_type)
    ↓
CellManager generates UUID, creates cell data, saves via DataStore
    ↓
CellManager returns cell_id
    ↓
NotebookManager adds cell_id to notebook's cell_ids list
    ↓
NotebookManager saves notebook via DataStore
    ↓
MainWindow calls NotebookView.add_cell(cell_id, position)
    ↓
NotebookView loads cell data, creates CodeCell widget
    ↓
CodeCell added to layout at specified position
    ↓
UI updated, cell appears on screen
```

### Selecting a Cell

```
User clicks on a cell
    ↓
Cell.mousePressEvent() triggered
    ↓
Cell calls self.setSelected(True)
    ↓
Cell emits selected(cell_id, cell_type) signal
    ↓
NotebookView receives signal
    ↓
NotebookView deselects previous cell
    ↓
NotebookView emits cellSelected(cell_id, cell_type) signal
    ↓
MainWindow receives signal
    ↓
MainWindow calls toolbar_stack.setCurrentWidget(code_toolbar)
    ↓
Toolbar switches to CodeToolbar
    ↓
UI updated, context-sensitive toolbar displayed
```

### Moving a Cell Up

```
User clicks "Move Up" in toolbar
    ↓
CodeToolbar emits move_cell_up(cell_id) signal
    ↓
MainWindow receives signal
    ↓
MainWindow calls NotebookManager.move_cell(notebook_id, cell_id, -1)
    ↓
NotebookManager updates cell_ids order in notebook data
    ↓
NotebookManager saves notebook via DataStore
    ↓
NotebookManager emits cell_order_changed(notebook_id) signal
    ↓
MainWindow calls NotebookView.reorder_cells(new_order)
    ↓
NotebookView rearranges cell widgets in layout
    ↓
UI updated, cell visually moved up
```

### Saving and Loading

```
Application Startup:
    DataStore loads from ~/.lunaqt/
    NotebookManager lists available notebooks
    User opens notebook
    NotebookManager loads notebook JSON
    For each cell_id in cell_ids:
        CellManager loads cell JSON
        NotebookView creates appropriate cell widget
    UI displays notebook with all cells

On Edit:
    User types in cell
    Cell emits content_changed(cell_id, new_content)
    Auto-save timer triggers
    CellManager updates cell data
    DataStore saves cell JSON
    
On Close:
    MainWindow calls NotebookManager.close_notebook()
    NotebookManager ensures all cells saved
    NotebookManager saves notebook state
    Application exits cleanly
```

---

## Implementation Phases (MVP-first, minimal)

### Phase 1: MVP shell (1–2 weeks)
- [x] QPalette theme system with semantic tokens and minimal QSS
- [x] ThemeManager applies palette and QSS
- [x] MainWindow with a resizable settings side panel (dock or splitter)
- [x] Basic settings controls (combobox/spinbox) styled and working
- [x] Future-ready folders and READMEs (models, managers, notebook UI)
- [ ] Persist window geometry/state (QSettings)

Acceptance: app launches cleanly; theme switching works; settings panel resizes smoothly; no hard-coded colors.

### Phase 2: Base UI Components (lightweight)
- [ ] Create NotebookView container (QListWidget-based placeholder)
- [ ] Add notebook_qss.py shell for future structural styles
- [ ] Decide dock vs splitter for settings (splitter recommended for stable MVP resize)

### Phase 3: Minimal data layer
- [ ] Introduce schema_version in models (even if minimal)
- [ ] Implement DataStore MVP (JSON read/write, atomic save)
- [ ] Implement simple NotebookManager API (create/open/save)
- [ ] UUID helper in utils

### Phase 4: Toolbars & Commands (post-MVP)
- [ ] Define a centralized command registry (IDs, labels, shortcuts)
- [ ] Wire menubar and shortcuts to registry
- [ ] Command palette shell (dialog + search) that consumes the registry

### Phase 5: Integration polish
- [ ] Auto-save with debounce
- [ ] Persist UI layout, recent files, last notebook
- [ ] Basic unit tests for DataStore and NotebookManager

### Phase 6: Advanced Features (Future)
- [ ] Code execution (Python kernel)
- [ ] Output capture and display
- [ ] Syntax highlighting improvements
- [ ] Code completion
- [ ] LaTeX rendering in markdown
- [ ] Export to HTML/PDF
- [ ] Collaborative editing
- [ ] Extensions/plugins system

---

## Testing Strategy (minimal now, expand later)

### Unit Tests
- Data models serialization/deserialization
- Manager operations (create, update, delete)
- DataStore file operations
- ID generation uniqueness

### Integration Tests
- Cell creation → UI display
- Cell reordering → data persistence
- Theme changes → style updates
- Notebook save/load roundtrip

### UI Tests
- Cell selection behavior
- Toolbar switching
- Keyboard navigation
- Drag-and-drop cell reordering (future)

---

## Performance Considerations (pragmatic for MVP)

1. **Lazy Loading**: Load cell data only when notebook opened
2. **Virtual Scrolling**: For notebooks with 100+ cells (future)
3. **Debounced Auto-save**: Save after 500ms of inactivity
4. **Cell Data Caching**: Keep recently accessed cells in memory
5. **Incremental Rendering**: Don't render markdown until visible

---

## Security Considerations (later)

1. **Code Execution Sandboxing**: Run code in restricted environment (future)
2. **File Path Validation**: Prevent directory traversal attacks
3. **Input Sanitization**: Sanitize markdown before rendering
4. **JSON Validation**: Validate data structure on load
5. **Permission Checks**: Verify file read/write permissions

---

## Accessibility (later)

1. **Keyboard Navigation**: Full keyboard support for all actions
2. **Screen Reader Support**: Proper ARIA labels (Qt accessibility)
3. **High Contrast Themes**: Ensure sufficient contrast ratios
4. **Configurable Fonts**: Respect user font size preferences
5. **Focus Indicators**: Clear visual focus indicators

---

## Future Enhancements

1. **Multiple Kernels**: Support Julia, R, JavaScript
2. **Rich Outputs**: Interactive plots, widgets
3. **Version Control**: Git integration for notebooks
4. **Collaboration**: Real-time collaborative editing
5. **Extensions API**: Plugin system for custom cell types
6. **Cloud Sync**: Sync notebooks across devices
7. **Template Library**: Notebook templates for common tasks
8. **AI Assistance**: Code completion, cell suggestions

---

## Best Practices Alignment

This plan adheres to `lunaqt/lunaqt_best_practices.md`:
- Keep functions small and single-responsibility; avoid one-liners
- Use snake_case for modules/functions/variables, PascalCase for classes, ALL_CAPS for constants
- Add type hints throughout and run mypy; use @dataclass for structured data as models land
- Use black for formatting and ruff for linting/import sorting

## Summary

This architecture provides:

✅ **Maintainability**: Base templates and centralized styling  
✅ **Scalability**: Manager layer handles complexity, QListWidget provides path to virtualization  
✅ **Consistency**: Theme integration throughout  
✅ **Flexibility**: Easy to add new cell types and features  
✅ **Performance**: Efficient data structure with ID references, semi-virtualized rendering  
✅ **User Experience**: Reactive UI with context-sensitive toolbars and command palette  
✅ **Future-proof**: Schema versioning, transaction safety, and upgrade paths planned  

The separation between data (models + managers) and presentation (UI components) ensures clean code that's easy to test, extend, and maintain.

### Key Architectural Decisions

1. **Hybrid List-Based Rendering**: QListWidget strikes balance between simplicity and performance
2. **Command Palette**: Enhances discoverability and keyboard-first workflows
3. **Base Template Pattern**: Ensures consistency across cells and toolbars
4. **QPalette + Minimal QSS**: Works with Qt's style engine, reduces code, better performance ⭐
5. **Semantic Color Tokens**: Maintainable, self-documenting color vocabulary
6. **Schema Versioning**: Enables safe format evolution (to be implemented in Phase 1)

### Implementation Strategy

- **Phase 1-2 (4 weeks)**: Foundation (data layer + base UI components)
- **Phase 3-4 (4 weeks)**: Cell types + toolbars + command palette
- **Phase 5 (2 weeks)**: Integration + polish
- **Phase 6 (ongoing)**: Advanced features (execution, Jupyter compat, extensions)

This plan balances ambition with pragmatism—building a solid MVP while leaving room for advanced features as the application matures.
