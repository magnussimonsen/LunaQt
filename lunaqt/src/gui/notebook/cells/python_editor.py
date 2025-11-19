"""Lightweight Python code editor with syntax highlighting and helpers."""

from __future__ import annotations

import re
from PySide6.QtCore import Qt, Signal, QRect, QSize
from PySide6.QtGui import (
    QColor,
    QFont,
    QPainter,
    QPalette,
    QSyntaxHighlighter,
    QTextCharFormat,
    QTextCursor,
    QTextDocument,
)
from PySide6.QtWidgets import QPlainTextEdit, QTextEdit, QWidget


class _PythonSyntaxHighlighter(QSyntaxHighlighter):
    """Simple Python syntax highlighter using regular expressions."""

    _KEYWORDS = {
        "False", "class", "finally", "is", "return",
        "None", "continue", "for", "lambda", "try",
        "True", "def", "from", "nonlocal", "while",
        "and", "del", "global", "not", "with",
        "as", "elif", "if", "or", "yield",
        "assert", "else", "import", "pass",
        "break", "except", "in", "raise",
    }

    _BUILTINS = {
        "abs", "all", "any", "bool", "dict", "float", "int",
        "len", "list", "map", "max", "min", "print", "range",
        "set", "str", "sum", "zip", "enumerate", "isinstance",
    }

    def __init__(self, document: QTextDocument) -> None:
        super().__init__(document)
        self._keyword_format = QTextCharFormat()
        self._keyword_format.setForeground(QColor(52, 120, 235))
        self._keyword_format.setFontWeight(QFont.Weight.Bold)

        self._builtin_format = QTextCharFormat()
        self._builtin_format.setForeground(QColor(169, 120, 227))

        self._comment_format = QTextCharFormat()
        self._comment_format.setForeground(QColor(120, 120, 120))
        self._comment_format.setFontItalic(True)

        self._string_format = QTextCharFormat()
        self._string_format.setForeground(QColor(220, 120, 70))

        self._defclass_format = QTextCharFormat()
        self._defclass_format.setForeground(QColor(50, 160, 100))
        self._defclass_format.setFontWeight(QFont.Weight.Bold)

        self._triple_single = re.compile(r"'''\w?", re.DOTALL)
        self._triple_double = re.compile(r'"""\w?', re.DOTALL)

    def highlightBlock(self, text: str) -> None:  # type: ignore[override]
        self.setCurrentBlockState(0)

        for match in re.finditer(r"\b[A-Za-z_][A-Za-z0-9_]*\b", text):
            word = match.group(0)
            if word in self._KEYWORDS:
                self.setFormat(match.start(), len(word), self._keyword_format)
            elif word in self._BUILTINS:
                self.setFormat(match.start(), len(word), self._builtin_format)

        for match in re.finditer(r"\b(def|class)\s+(\w+)", text):
            self.setFormat(match.start(1), len(match.group(1)), self._keyword_format)
            self.setFormat(match.start(2), len(match.group(2)), self._defclass_format)

        for match in re.finditer(r"#[^\n]*", text):
            self.setFormat(match.start(), match.end() - match.start(), self._comment_format)

        for match in re.finditer(r"(['\"])(?:(?=(\\?))\2.)*?\1", text):
            self.setFormat(match.start(), match.end() - match.start(), self._string_format)

        self._apply_multiline_strings(text)

    def _apply_multiline_strings(self, text: str) -> None:
        # Track triple quoted strings over multiple blocks
        for delimiter, pattern in (("'''", self._triple_single), ('"""', self._triple_double)):
            start_idx = 0
            if self.previousBlockState() == hash(delimiter):
                start_idx = 0
                end_idx = text.find(delimiter, start_idx)
                if end_idx == -1:
                    self.setFormat(start_idx, len(text) - start_idx, self._string_format)
                    self.setCurrentBlockState(hash(delimiter))
                    return
                self.setFormat(start_idx, end_idx - start_idx + 3, self._string_format)
                start_idx = end_idx + 3

            while True:
                start_idx = text.find(delimiter, start_idx)
                if start_idx < 0:
                    break
                end_idx = text.find(delimiter, start_idx + 3)
                if end_idx < 0:
                    self.setFormat(start_idx, len(text) - start_idx, self._string_format)
                    self.setCurrentBlockState(hash(delimiter))
                    return
                self.setFormat(start_idx, end_idx - start_idx + 3, self._string_format)
                start_idx = end_idx + 3


class _LineNumberArea(QWidget):
    """Side widget used for rendering editor line numbers."""

    def __init__(self, editor: 'PythonCodeEditor') -> None:
        super().__init__(editor)
        self._editor = editor

    def sizeHint(self):  # type: ignore[override]
        return QSize(self._editor._line_number_area_width(), 0)

    def paintEvent(self, event):  # type: ignore[override]
        self._editor._paint_line_numbers(event)


class PythonCodeEditor(QPlainTextEdit):
    """Plain text editor tuned for Python editing."""

    focus_changed = Signal(bool)
    execute_requested = Signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._tab_spaces = 4
        self.setTabChangesFocus(False)
        self.setLineWrapMode(QPlainTextEdit.LineWrapMode.NoWrap)
        self.setFrameShape(QPlainTextEdit.Shape.NoFrame)
        self.setFrameShadow(QPlainTextEdit.Shadow.Plain)
        self._highlighter = _PythonSyntaxHighlighter(self.document())
        self._line_number_area = _LineNumberArea(self)
        self.blockCountChanged.connect(self._update_line_number_area_width)
        self.updateRequest.connect(self._update_line_number_area)
        self._update_line_number_area_width(0)
        self.cursorPositionChanged.connect(self._highlight_current_line)
        self._highlight_current_line()

    # Focus notifications -------------------------------------------------
    def focusInEvent(self, event):  # type: ignore[override]
        super().focusInEvent(event)
        self.focus_changed.emit(True)

    def focusOutEvent(self, event):  # type: ignore[override]
        super().focusOutEvent(event)
        self.focus_changed.emit(False)

    # Tab/Enter handling --------------------------------------------------
    def set_tab_width_spaces(self, spaces: int) -> None:
        self._tab_spaces = max(1, spaces)
        metrics = self.fontMetrics()
        self.setTabStopDistance(metrics.horizontalAdvance(' ') * self._tab_spaces)

    def keyPressEvent(self, event):  # type: ignore[override]
        key = event.key()
        modifiers = event.modifiers()

        if key in (Qt.Key_Return, Qt.Key_Enter) and modifiers == Qt.ShiftModifier:
            event.accept()
            self.execute_requested.emit()
            return

        if key in (Qt.Key_Return, Qt.Key_Enter):
            indent = self._current_line_indent()
            extra = ''
            if self.textCursor().block().text().rstrip().endswith(":"):
                extra = ' ' * self._tab_spaces
            super().keyPressEvent(event)
            self.insertPlainText(indent + extra)
            return

        if key == Qt.Key_Backspace and modifiers == Qt.NoModifier:
            cursor = self.textCursor()
            if not cursor.hasSelection():
                pos_in_block = cursor.positionInBlock()
                if pos_in_block >= self._tab_spaces:
                    block_text = cursor.block().text()
                    segment = block_text[pos_in_block - self._tab_spaces : pos_in_block]
                    if segment == ' ' * self._tab_spaces:
                        cursor.beginEditBlock()
                        for _ in range(self._tab_spaces):
                            cursor.deletePreviousChar()
                        cursor.endEditBlock()
                        return
            super().keyPressEvent(event)
            return

        if key == Qt.Key_Tab and modifiers in (Qt.NoModifier, Qt.ShiftModifier):
            spaces = ' ' * self._tab_spaces
            cursor = self.textCursor()
            if modifiers == Qt.ShiftModifier:
                cursor.movePosition(QTextCursor.MoveOperation.StartOfLine, QTextCursor.MoveAnchor)
                line = cursor.block().text()
                remove = min(self._tab_spaces, len(line) - len(line.lstrip(' ')))
                if remove > 0:
                    cursor.movePosition(QTextCursor.MoveOperation.Right, QTextCursor.KeepAnchor, remove)
                    cursor.removeSelectedText()
                else:
                    self.textCursor().deletePreviousChar()
            else:
                cursor.insertText(spaces)
            return

        super().keyPressEvent(event)

    # Helpers --------------------------------------------------------------
    def _line_number_area_width(self) -> int:
        digits = len(str(max(1, self.blockCount())))
        # Extra padding for readability
        space = self.fontMetrics().horizontalAdvance('9') * digits + 12
        return space

    def _update_line_number_area_width(self, _new_block_count: int) -> None:
        margins = self.viewportMargins()
        left_margin = self._line_number_area_width()
        if margins.left() != left_margin:
            self.setViewportMargins(left_margin, margins.top(), margins.right(), margins.bottom())

    def _update_line_number_area(self, rect, dy) -> None:
        if dy:
            self._line_number_area.scroll(0, dy)
        else:
            self._line_number_area.update(0, rect.y(), self._line_number_area.width(), rect.height())

        if rect.contains(self.viewport().rect()):
            self._update_line_number_area_width(self.blockCount())

    def resizeEvent(self, event):  # type: ignore[override]
        super().resizeEvent(event)
        cr = self.contentsRect()
        width = self._line_number_area_width()
        self._line_number_area.setGeometry(QRect(cr.left(), cr.top(), width, cr.height()))

    def _paint_line_numbers(self, event) -> None:
        painter = QPainter(self._line_number_area)
        bg = self.palette().color(QPalette.ColorRole.Base).darker(110)
        fg = self.palette().color(QPalette.ColorRole.Text)
        highlight = self.palette().color(QPalette.ColorRole.Highlight)
        painter.fillRect(event.rect(), bg)

        block = self.firstVisibleBlock()
        block_number = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())
        current = self.textCursor().blockNumber()

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(block_number + 1)
                painter.setPen(highlight if block_number == current else fg)
                painter.drawText(
                    0,
                    top,
                    self._line_number_area.width() - 6,
                    self.fontMetrics().height(),
                    Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter,
                    number,
                )

            block = block.next()
            block_number += 1
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())

    def _current_line_indent(self) -> str:
        cursor = self.textCursor()
        line_text = cursor.block().text()
        match = re.match(r"\s*", line_text)
        return match.group(0) if match else ""

    def _highlight_current_line(self) -> None:
        extra = []
        if not self.isReadOnly():
            selection = QTextEdit.ExtraSelection()
            line_color = QColor(80, 80, 80, 40)
            selection.format.setBackground(line_color)
            selection.format.setProperty(QTextCharFormat.FullWidthSelection, True)
            selection.cursor = self.textCursor()
            selection.cursor.clearSelection()
            extra.append(selection)
        self.setExtraSelections(extra)

    def set_font(self, font: QFont) -> None:
        self.setFont(font)
        self.set_tab_width_spaces(self._tab_spaces)