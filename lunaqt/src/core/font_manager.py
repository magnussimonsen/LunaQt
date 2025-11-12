"""Font management system for loading and registering custom fonts.

This module handles loading custom fonts from the assets/fonts directory
and makes them available throughout the application.
"""

import logging
from pathlib import Path
from typing import List
from PySide6.QtGui import QFontDatabase

logger = logging.getLogger(__name__)


class FontManager:
    """Manages custom font loading and registration."""
    
    def __init__(self) -> None:
        """Initialize the font manager."""
        self.loaded_fonts: List[str] = []
        self.font_families: List[str] = []
    
    def load_all_fonts(self) -> None:
        """Load all custom fonts from the assets/fonts directory.
        
        This scans the fonts directory and loads all .otf and .ttf files.
        """
        # Get the fonts directory path
        fonts_dir = Path(__file__).parent.parent / "assets" / "fonts"
        
        if not fonts_dir.exists():
            logger.warning("Fonts directory not found: %s", fonts_dir)
            return
        
        # Load all .otf and .ttf files from all subdirectories
        font_extensions = ["*.otf", "*.ttf"]
        
        for extension in font_extensions:
            for font_file in fonts_dir.rglob(extension):
                # Skip temporary download folder
                if "temp-font-downloads" in str(font_file):
                    continue
                    
                self._load_font(font_file)
    
    def _load_font(self, font_path: Path) -> None:
        """Load a single font file.
        
        Args:
            font_path: Path to the font file
        """
        font_id = QFontDatabase.addApplicationFont(str(font_path))
        
        if font_id == -1:
            logger.warning("Failed to load font: %s", font_path.name)
            return
        
        # Get the font families that were loaded
        families = QFontDatabase.applicationFontFamilies(font_id)
        
        for family in families:
            if family not in self.font_families:
                self.font_families.append(family)
                logger.info("Loaded font family: %s from %s", family, font_path.name)
        
        self.loaded_fonts.append(str(font_path))
    
    def get_available_fonts(self) -> List[str]:
        """Get list of all loaded custom font families.
        
        Returns:
            List of font family names
        """
        return self.font_families.copy()
    
    def is_font_loaded(self, family_name: str) -> bool:
        """Check if a specific font family is loaded.
        
        Args:
            family_name: Font family name to check
            
        Returns:
            True if the font family is loaded
        """
        return family_name in self.font_families


# Global font manager instance
_font_manager: FontManager | None = None


def get_font_manager() -> FontManager:
    """Get the global font manager instance.
    
    Returns:
        FontManager instance
    """
    global _font_manager
    if _font_manager is None:
        _font_manager = FontManager()
    return _font_manager


def initialize_fonts() -> None:
    """Initialize and load all custom fonts.
    
    Call this once at application startup.
    """
    manager = get_font_manager()
    manager.load_all_fonts()
