# LunaQt Notebook Architecture Plan

## Overview

LunaQt will implement a Jupyter-like notebook system using PySide6 (Qt6) with a focus on maintainability, theme integration, and clean separation of concerns.

---

## Data Structure Architecture

### Core Principles

1. **Unique IDs**: Every cell and notebook has a unique identifier (UUID)
2. **JSON Storage**: All cell data persisted in JSON format
3. **ID-based References**: Notebooks store ordered lists of cell IDs (not cell data)
4. **Separation**: Data layer completely independent of UI layer

### Data Models

#### Cell Model (`models/cell.py`)

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
- `markdown`: Formatted text with Markdown
- `raw`: Plain text, no execution or rendering

#### Notebook Model (`models/notebook.py`)

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

**Key Feature**: Reordering cells only modifies the `cell_ids` array order—no cell data duplication.

---

## Storage Architecture

### File Structure

```
~/.lunaqt/
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

**Responsibilities:**
- Load/save individual cells from JSON
- Load/save notebooks from JSON
- Handle file I/O and error handling
- Provide atomic write operations
- Cache recently accessed cells

**Key Methods:**
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

## Manager Layer

### Cell Manager (`core/cell_manager.py`)

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

### Notebook Manager (`core/notebook_manager.py`)

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

### Cell Executor (`core/cell_executor.py`)

**Responsibilities (Future):**
- Execute code cells
- Capture output (stdout, stderr, results)
- Manage kernel/interpreter
- Handle execution state
- Interrupt/restart execution

---

## UI Architecture

### Component Hierarchy

```
MainWindow (QMainWindow)
├── MenuBar
├── ToolbarStack (QStackedWidget)
│   ├── DefaultToolbar (no cell selected)
│   ├── CodeToolbar (code cell selected)
│   └── MarkdownToolbar (markdown cell selected)
└── NotebookView (QWidget)
    └── QScrollArea
        └── CellContainer (QWidget)
            └── QVBoxLayout
                ├── CodeCell (inherits BaseCell)
                ├── MarkdownCell (inherits BaseCell)
                └── CodeCell (inherits BaseCell)
```

### Base Components

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

### Cell Types

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

### Toolbar Types

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

## Styling System

### Cell Styles (`gui/notebook/styles/cell_stylesheet.py`)

**Integration:** Reads from `themes/colors.py`

**Style Classes:**
- `CellStylesheet.get_base_style(theme_colors)` - Common cell styles
- `CellStylesheet.get_selected_style(theme_colors)` - Selection highlight
- `CellStylesheet.get_code_cell_style(theme_colors)` - Code-specific styles
- `CellStylesheet.get_markdown_cell_style(theme_colors)` - Markdown-specific styles

**Theme Properties Used:**
```python
background, background_selected
text, code_text
border, border_hover
accent (selection color)
code_background
```

### Toolbar Styles (`gui/notebook/styles/toolbar_stylesheet.py`)

**Integration:** Reads from `themes/colors.py`

**Style Class:**
- `ToolbarStylesheet.get_base_style(theme_colors)` - All toolbar styles

**Styled Elements:**
- QPushButton (standard buttons)
- QToolButton (icon buttons)
- Hover states
- Pressed states

**Theme Properties Used:**
```python
toolbar_background
button_background, button_hover, button_pressed
button_border
```

### Theme Updates

**Flow:**
1. User changes theme in settings
2. `ThemeManager` emits `theme_changed` signal
3. `MainWindow` receives signal
4. `MainWindow` calls `updateNotebookStyles()`
5. New stylesheets generated with new theme colors
6. Applied to all cells and toolbars

---

## Project Structure

```
lunaqt/src/
├── constants/
│   └── notebook_constants.py          # Cell types, defaults, configs
│
├── models/
│   ├── __init__.py
│   ├── notebook.py                    # Notebook data class
│   └── cell.py                        # Cell data classes
│
├── core/
│   ├── notebook_manager.py            # Notebook CRUD & ordering
│   ├── cell_manager.py                # Cell CRUD & conversions
│   ├── data_store.py                  # JSON persistence layer
│   └── cell_executor.py               # Code execution (future)
│
├── gui/
│   ├── main_window.py                 # Main application window
│   │
│   └── notebook/
│       ├── __init__.py
│       ├── notebook_view.py           # Cell container & orchestration
│       │
│       ├── cells/
│       │   ├── __init__.py
│       │   ├── base_cell.py           # ⭐ Cell template
│       │   ├── code_cell.py           # Code editor cell
│       │   ├── markdown_cell.py       # Markdown cell
│       │   └── output_widget.py       # Output display
│       │
│       ├── toolbars/
│       │   ├── __init__.py
│       │   ├── base_toolbar.py        # ⭐ Toolbar template
│       │   ├── default_toolbar.py     # General notebook actions
│       │   ├── code_toolbar.py        # Code cell actions
│       │   └── markdown_toolbar.py    # Markdown cell actions
│       │
│       └── styles/
│           ├── __init__.py
│           ├── cell_stylesheet.py     # ⭐ Cell style generator
│           └── toolbar_stylesheet.py  # ⭐ Toolbar style generator
│
└── utils/
    ├── __init__.py
    └── id_generator.py                # UUID generation
```

---

## Key Design Patterns

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

## Data Flow Examples

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

## Implementation Phases

### Phase 1: Foundation (Weeks 1-2)
- [ ] Create data models (`models/cell.py`, `models/notebook.py`)
- [ ] Implement `DataStore` (JSON read/write)
- [ ] Create `CellManager` and `NotebookManager`
- [ ] Write unit tests for data layer
- [ ] Implement ID generation utility

### Phase 2: Base UI Components (Weeks 3-4)
- [ ] Create `BaseCell` template
- [ ] Create `BaseToolbar` template
- [ ] Implement cell and toolbar stylesheets
- [ ] Integrate with existing theme system
- [ ] Create `NotebookView` container

### Phase 3: Cell Types (Weeks 5-6)
- [ ] Implement `CodeCell` with basic text editor
- [ ] Implement `MarkdownCell` with edit/preview modes
- [ ] Add basic syntax highlighting for code
- [ ] Implement markdown rendering
- [ ] Create `OutputWidget` for cell outputs

### Phase 4: Toolbars & Actions (Week 7)
- [ ] Implement `DefaultToolbar`
- [ ] Implement `CodeToolbar`
- [ ] Implement `MarkdownToolbar`
- [ ] Connect toolbar actions to managers
- [ ] Add keyboard shortcuts

### Phase 5: Integration (Week 8)
- [ ] Integrate into `MainWindow`
- [ ] Connect all signals and slots
- [ ] Implement auto-save
- [ ] Add notebook file browser
- [ ] Polish UI and fix bugs

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

## Testing Strategy

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

## Performance Considerations

1. **Lazy Loading**: Load cell data only when notebook opened
2. **Virtual Scrolling**: For notebooks with 100+ cells (future)
3. **Debounced Auto-save**: Save after 500ms of inactivity
4. **Cell Data Caching**: Keep recently accessed cells in memory
5. **Incremental Rendering**: Don't render markdown until visible

---

## Security Considerations

1. **Code Execution Sandboxing**: Run code in restricted environment (future)
2. **File Path Validation**: Prevent directory traversal attacks
3. **Input Sanitization**: Sanitize markdown before rendering
4. **JSON Validation**: Validate data structure on load
5. **Permission Checks**: Verify file read/write permissions

---

## Accessibility

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

## Summary

This architecture provides:

✅ **Maintainability**: Base templates and centralized styling  
✅ **Scalability**: Manager layer handles complexity  
✅ **Consistency**: Theme integration throughout  
✅ **Flexibility**: Easy to add new cell types and features  
✅ **Performance**: Efficient data structure with ID references  
✅ **User Experience**: Reactive UI with context-sensitive toolbars  

The separation between data (models + managers) and presentation (UI components) ensures clean code that's easy to test, extend, and maintain.
