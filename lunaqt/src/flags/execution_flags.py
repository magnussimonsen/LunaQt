"""Execution-related feature flags."""

from __future__ import annotations

import os

# Default keeps the UI clean; set to True if you want indicators on by default.
_DEFAULT_ENABLED = False
# Env var allows toggling without code changes ("0/false/off/no" disable).
_ENV_VAR = "LUNAQT_SHOW_EXECUTION_INDICATOR"

ENABLE_EXECUTION_INDICATOR = (
    os.getenv(_ENV_VAR, "1").lower() not in {"0", "false", "off", "no"}
    if os.getenv(_ENV_VAR) is not None
    else _DEFAULT_ENABLED
)


def execution_indicator_enabled() -> bool:
    """Return whether the execution gutter indicator should be rendered."""
    return ENABLE_EXECUTION_INDICATOR
