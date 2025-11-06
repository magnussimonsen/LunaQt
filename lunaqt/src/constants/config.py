"""Type-safe configuration using dataclasses."""

from dataclasses import dataclass

from .types import ThemeMode


@dataclass
class WindowConfig:
    """Main window configuration."""
    
    width: int
    height: int
    min_width: int
    min_height: int
    title: str
    theme: ThemeMode = "light"
    
    def __post_init__(self) -> None:
        """Validate configuration after initialization."""
        if self.width < self.min_width:
            raise ValueError(f"Width {self.width} cannot be less than min_width {self.min_width}")
        if self.height < self.min_height:
            raise ValueError(f"Height {self.height} cannot be less than min_height {self.min_height}")


@dataclass
class AppConfig:
    """Application configuration."""
    
    name: str
    version: str
    author: str
    license: str
    description: str = ""
    
    def get_full_title(self) -> str:
        """Get full application title with version."""
        return f"{self.name} v{self.version}"
