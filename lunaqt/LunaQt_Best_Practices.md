# LunaQt Best Practices

## Unix Philosophy
Do one thing and do it well.

## Avoid one-liners
Code is read far more than it is written. Avoid cramming multiple operations into a single, cryptic expression; instead, favor clear, step-by-step code with meaningful names. This makes logic easier to review, debug, and modify later.

## Naming Conventions

- **Functions:** `snake_case` (e.g., `calculate_determinant`)
- **Variables:** `snake_case` (e.g., `user_input`)
- **Classes:** `PascalCase` (e.g., `MatrixCalculator`)
- **Constants:** `ALL_CAPS` (e.g., `DEFAULT_PRECISION`)
- **Files:** `snake_case.py` (e.g., `matrix_ops.py`)
- **Folders:** `snake_case` (e.g., `my_folder`)

## Type Safety

Use **mypy** for static type checking.  
Add type hints everywhere and run `mypy` regularly.

Example (Python 3.9+):

```python
def calculate_determinant(matrix: list[list[float]]) -> float:
    """Calculate matrix determinant."""
    return 0.0
```

## Code Rules

- One function = one responsibility  
- Use descriptive, explicit names  
- Keep functions "small" 
- Add type hints everywhere  
- Run mypy for type checking  
- Use `@dataclass` for structured data  

## Code Quality Tools

- **black** – automatic formatting  
- **ruff** – linting and import sorting  
- **mypy** – static type checking  

## Documentation

Write concise docstrings for all public classes and functions.  
Follow Google or NumPy docstring style consistently.
