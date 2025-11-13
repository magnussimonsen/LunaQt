"""Base toolbar widget for notebook cells."""

from PySide6.QtWidgets import QWidget, QHBoxLayout, QLabel, QPushButton
from PySide6.QtCore import Signal


class BaseToolbar(QWidget):
    """Base toolbar widget that can be populated with buttons.
    
    Provides a horizontal layout for buttons and other toolbar items.
    Subclasses can add specific buttons for different cell types.
    """
    
    def __init__(self, parent: QWidget | None = None) -> None:
        """Initialize base toolbar.
        
        Args:
            parent: Parent widget
        """
        super().__init__(parent)
        self.setObjectName("BaseToolbar")
        
        # Horizontal layout for toolbar items
        self.layout = QHBoxLayout(self)
        self.layout.setContentsMargins(4, 2, 4, 2)
        self.layout.setSpacing(6)
        
    def clear_buttons(self) -> None:
        """Remove all items from the toolbar."""
        while self.layout.count():
            item = self.layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
    
    def add_button(self, text: str, callback=None) -> QPushButton:
        """Add a button to the toolbar.
        
        Args:
            text: Button text
            callback: Optional callback function when button is clicked
            
        Returns:
            The created button
        """
        button = QPushButton(text)
        if callback:
            button.clicked.connect(callback)
        self.layout.addWidget(button)
        return button
    
    def add_label(self, text: str) -> QLabel:
        """Add a label to the toolbar.
        
        Args:
            text: Label text
            
        Returns:
            The created label
        """
        label = QLabel(text)
        self.layout.addWidget(label)
        return label
    
    def add_stretch(self) -> None:
        """Add a stretch to push subsequent items to the right."""
        self.layout.addStretch()
