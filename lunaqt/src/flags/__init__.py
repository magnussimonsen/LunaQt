"""Feature flag helpers for LunaQt."""

from .execution_flags import ENABLE_EXECUTION_INDICATOR, execution_indicator_enabled

__all__ = [
    "ENABLE_EXECUTION_INDICATOR",
    "execution_indicator_enabled",
]
