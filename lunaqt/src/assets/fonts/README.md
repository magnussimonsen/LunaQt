# Fonts Directory

This directory contains fonts used in the application.

## Structure

```
fonts/
├── font_lists.py          # Configuration of system and bundled fonts
├── OpenDyslexic/          # Bundled accessible font
│   ├── OpenDyslexic-Regular.otf
│   ├── OpenDyslexic-Bold.otf
│   ├── OpenDyslexic-Italic.otf
│   └── OpenDyslexic-Bold-Italic.otf
├── AnotherFont/           # Add more fonts here
│   ├── AnotherFont-Regular.ttf
│   └── AnotherFont-Bold.ttf
└── temp-font-downloads/   # Ignored by font loader
```

## Font Categories

### 1. System Fonts
Standard fonts available on most operating systems (Arial, Helvetica, etc.)
- Defined in `font_lists.py` as `SYSTEM_FONTS`
- Not included in repository

### 2. Bundled Fonts
Third-party fonts that ship with the application
- Defined in `font_lists.py` as `BUNDLED_FONTS`
- Stored in subdirectories (e.g., `OpenDyslexic/`)
- Automatically loaded at startup

### 3. Custom Fonts
User-added fonts loaded from subdirectories
- Not listed in `font_lists.py`
- Automatically discovered and loaded

## Supported Formats

- `.otf` (OpenType Font)
- `.ttf` (TrueType Font)

## Adding New Fonts

### To add a bundled font (ships with app):
1. Create a subdirectory: `fonts/YourFontName/`
2. Copy font files (.otf or .ttf) into that directory
3. Add the font family name to `BUNDLED_FONTS` in `font_lists.py`
4. Restart the application

### To add fonts for development/testing:
1. Create a subdirectory: `fonts/YourFontName/`
2. Copy font files (.otf or .ttf) into that directory
3. Restart the application (no code changes needed)

## Bundled Fonts

### OpenDyslexic
An accessible font designed for dyslexic readers.

- **License**: SIL Open Font License (OFL)
- **Website**: https://opendyslexic.org/
- **Files**: Regular, Bold, Italic, Bold-Italic variants
- **Purpose**: Accessibility feature for users with dyslexia

## Notes

- Font files in `temp-font-downloads/` are ignored by the font loader
- Fonts are loaded once at application startup
- After adding fonts, restart the application to see them in the UI
- System fonts are listed for reference but loaded from the OS
