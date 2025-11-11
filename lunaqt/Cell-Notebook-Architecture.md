
## Architecture: Cell Movement Logic

The notebook UI implements a **persistence-first** approach for cell reordering operations:

### Update Persistence + Reload Pattern

When a cell is moved (up/down):

1. **Update the persistence layer**: The notebook manager's cell order is modified to reflect the new position
2. **Reload the UI from persistence**: The entire cell list is rebuilt from the updated data
3. **Select the moved cell**: Focus is restored to the cell at its new position

### Why This Design Works

- **Single source of truth**: The persistence layer (notebook manager) is the authoritative source for cell order
- **No widget manipulation**: Cell widgets are never directly moved, swapped, or reparented during reordering
- **Clean slate**: Each reload creates fresh widget instances, eliminating stale references and Qt parent/ownership conflicts
- **Simplicity**: The move operation is ~30 lines vs. 150+ lines of complex widget juggling
- **Reliability**: No risk of widget invalidation, blank cells, or crashes due to Qt internal state inconsistencies

This pattern applies the principle: **modify data, then re-render** — avoiding brittle imperative DOM/widget manipulation in favor of declarative reconstruction from authoritative state.