"""Matplotlib style presets that mirror the app themes."""

from __future__ import annotations

MatplotlibStyle = dict[str, str]

DEFAULT_MPL_STYLE_NAME = "light"

MATPLOTLIB_STYLES: dict[str, MatplotlibStyle] = {
    "light": {
        "figure.facecolor": "#ffffff",
        "axes.facecolor": "#ffffff",
        "axes.edgecolor": "#000000",
        "text.color": "#000000",
        "axes.labelcolor": "#000000",
        "xtick.color": "#000000",
        "ytick.color": "#000000",
        "grid.color": "#d4d4d4",
    },
    "dark": {
        "figure.facecolor": "#1e1e1e",
        "axes.facecolor": "#1e1e1e",
        "axes.edgecolor": "#f0f0f0",
        "text.color": "#f0f0f0",
        "axes.labelcolor": "#f0f0f0",
        "xtick.color": "#f0f0f0",
        "ytick.color": "#f0f0f0",
        "grid.color": "#3c3c3c",
    },
}


def get_matplotlib_style(theme: str | None) -> MatplotlibStyle:
    """Return the rcParam overrides for the given theme name."""

    if theme and theme in MATPLOTLIB_STYLES:
        return MATPLOTLIB_STYLES[theme]
    return MATPLOTLIB_STYLES.get(DEFAULT_MPL_STYLE_NAME, {})
