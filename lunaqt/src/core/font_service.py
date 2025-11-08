"""FontService: central source of truth for app fonts with Qt signals.

Other windows/panels can subscribe to be notified when UI/Text/Code fonts
change, without depending on MainWindow directly.
"""
from __future__ import annotations

from typing import Optional
from PySide6.QtCore import QObject, Signal


class FontService(QObject):
    """Holds font settings and emits signals when they change."""

    # family, size
    uiFontChanged = Signal(str, int)
    textFontChanged = Signal(str, int)
    codeFontChanged = Signal(str, int)

    def __init__(self) -> None:
        super().__init__()
        self._ui_family: Optional[str] = None
        self._ui_size: Optional[int] = None
        self._text_family: Optional[str] = None
        self._text_size: Optional[int] = None
        self._code_family: Optional[str] = None
        self._code_size: Optional[int] = None

    # UI font
    def set_ui_font(self, family: str, size: int) -> None:
        if family != self._ui_family or size != self._ui_size:
            self._ui_family = family
            self._ui_size = size
            self.uiFontChanged.emit(family, size)

    def get_ui_font(self) -> tuple[Optional[str], Optional[int]]:
        return self._ui_family, self._ui_size

    # Text font
    def set_text_font(self, family: str, size: int) -> None:
        if family != self._text_family or size != self._text_size:
            self._text_family = family
            self._text_size = size
            self.textFontChanged.emit(family, size)

    def get_text_font(self) -> tuple[Optional[str], Optional[int]]:
        return self._text_family, self._text_size

    # Code font
    def set_code_font(self, family: str, size: int) -> None:
        if family != self._code_family or size != self._code_size:
            self._code_family = family
            self._code_size = size
            self.codeFontChanged.emit(family, size)

    def get_code_font(self) -> tuple[Optional[str], Optional[int]]:
        return self._code_family, self._code_size


_font_service_instance: Optional[FontService] = None


def get_font_service() -> FontService:
    global _font_service_instance
    if _font_service_instance is None:
        _font_service_instance = FontService()
    return _font_service_instance
