"""Central place to tweak widget-level shape/border rules."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict

from .semantic_colors import SemanticColors, ThemeMode


@dataclass(frozen=True)
class WidgetStateStyle:
    """Represents a pseudo-state override (hover, focus, etc.)."""

    background_token: str | None = None
    border_color_token: str | None = None
    text_color_token: str | None = None


@dataclass(frozen=True)
class WidgetStyle:
    """Declarative styling for a Qt selector."""

    selector: str
    border_radius: int | None = None
    border_width: int | None = 1
    border_color_token: str | None = "border.default"
    padding: str | None = None
    min_height: int | None = None
    max_height: int | None = None
    background_token: str | None = None
    text_color_token: str | None = None
    extra_rules: str = ""
    states: Dict[str, WidgetStateStyle] = field(default_factory=dict)

    def render(self, colors: Dict[str, str]) -> str:
        base_rules: list[str] = []
        if self.border_width is not None:
            if self.border_width == 0:
                base_rules.append("border: none;")
            elif self.border_color_token:
                base_rules.append(
                    f"border: {self.border_width}px solid {colors[self.border_color_token]};"
                )
        if self.border_radius is not None:
            base_rules.append(f"border-radius: {self.border_radius}px;")
        if self.padding:
            base_rules.append(f"padding: {self.padding};")
        if self.min_height is not None:
            base_rules.append(f"min-height: {self.min_height}px;")
        if self.background_token:
            base_rules.append(f"background-color: {colors[self.background_token]};")
        if self.text_color_token:
            base_rules.append(f"color: {colors[self.text_color_token]};")
        if self.extra_rules:
            base_rules.append(self.extra_rules.strip())

        base_body = "\n                ".join(base_rules) if base_rules else "/* palette defaults */"
        block = f"{self.selector} {{\n                {base_body}\n            }}\n"

        for suffix, state in self.states.items():
            state_rules: list[str] = []
            if state.background_token:
                state_rules.append(f"background-color: {colors[state.background_token]};")
            if state.border_color_token:
                state_rules.append(f"border-color: {colors[state.border_color_token]};")
            if state.text_color_token:
                state_rules.append(f"color: {colors[state.text_color_token]};")
            if not state_rules:
                continue
            body = "\n                ".join(state_rules)
            block += (
                f"{self.selector}{suffix} {{\n"
                f"                {body}\n"
                f"            }}\n"
            )

        return block.strip()


class WidgetStyling:
    """Registry for widget styling rules."""

    _STYLES: tuple[WidgetStyle, ...] = (
        WidgetStyle(
            selector="QPushButton",
            border_radius=4,
            padding="6px 8px",
            min_height=14,
            max_height=28,
            text_color_token="text.primary",
            states={
                ":hover": WidgetStateStyle(
                    background_token="action.hover",
                    border_color_token="action.hover",
                    text_color_token="text.inverted",
                ),
                ":pressed": WidgetStateStyle(
                    background_token="action.pressed",
                    border_color_token="action.pressed",
                    text_color_token="text.inverted",
                ),
                ":checked": WidgetStateStyle(
                    background_token="action.primary",
                    border_color_token="border.focus",
                    text_color_token="text.inverted",
                ),
            },
        ),
        WidgetStyle(
            selector="BaseToolbar QPushButton",
            padding="3px 10px",
            min_height=20,
            max_height=28,
        ),
        WidgetStyle(
            selector="QTextEdit",
            border_radius=6,
            border_color_token="border.subtle",
            padding="4px",
            background_token="code.background",
            text_color_token="code.text",
            states={
                ":focus": WidgetStateStyle(border_color_token="border.focus"),
            },
        ),
        WidgetStyle(
            selector="QPlainTextEdit",
            border_radius=6,
            border_color_token="border.subtle",
            padding="4px",
            background_token="code.background",
            text_color_token="code.text",
            states={
                ":focus": WidgetStateStyle(border_color_token="border.focus"),
            },
        ),
        WidgetStyle(
            selector="QLineEdit",
            border_radius=4,
            padding="4px",
            states={
                ":focus": WidgetStateStyle(border_color_token="border.focus"),
            },
        ),
        WidgetStyle(
            selector="QComboBox",
            border_radius=4,
            padding="4px 8px",
            states={
                ":hover": WidgetStateStyle(border_color_token="action.hover"),
            },
        ),
        WidgetStyle(
            selector="QSpinBox",
            border_radius=4,
            padding="4px",
            extra_rules="padding-right: 18px;",
            states={
                ":hover": WidgetStateStyle(border_color_token="action.hover"),
            },
        ),
    )

    @classmethod
    def build(cls, theme: ThemeMode) -> str:
        colors = SemanticColors.get_all(theme)
        blocks = [style.render(colors) for style in cls._STYLES]
        if not blocks:
            return ""
        indent = "            "
        indented_lines: list[str] = []
        for block in blocks:
            for line in block.splitlines():
                indented_lines.append(f"{indent}{line}")
        return "\n".join(indented_lines)
