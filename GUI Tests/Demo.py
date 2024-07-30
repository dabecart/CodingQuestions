import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QToolBar, QPushButton,
    QVBoxLayout, QWidget, QCheckBox, QLineEdit, QLabel
)
from PyQt6.QtCore import Qt

class SettingsWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('Settings')
        self.setGeometry(100, 100, 300, 200)
        
        layout = QVBoxLayout()

        # Add some fields and checkboxes
        self.text_field = QLineEdit(self)
        self.text_field.setPlaceholderText('Enter text here')
        layout.addWidget(QLabel('Text Field:'))
        layout.addWidget(self.text_field)

        self.checkbox1 = QCheckBox('Option 1', self)
        layout.addWidget(self.checkbox1)

        self.checkbox2 = QCheckBox('Option 2', self)
        layout.addWidget(self.checkbox2)
        
        self.checkbox3 = QCheckBox('Option 3', self)
        layout.addWidget(self.checkbox3)
        
        self.setLayout(layout)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('Main Window')
        self.setGeometry(100, 100, 600, 400)
        
        # Create a toolbar
        self.toolbar = QToolBar()
        self.addToolBar(self.toolbar)
        
        # Create actions for the toolbar
        self.settings_action = self.toolbar.addAction('Open Settings')
        self.settings_action.triggered.connect(self.open_settings_window)
        self.toolbar.addAction(self.settings_action)
        
        # Create a central widget and set it as the main window's central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout()
        central_widget.setLayout(layout)
        
        # Add a button to the main window
        self.button = QPushButton('Click Me', self)
        layout.addWidget(self.button)
        
    def open_settings_window(self):
        self.settings_window = SettingsWindow()
        self.settings_window.show()

def main():
    app = QApplication(sys.argv)
    main_window = MainWindow()
    main_window.show()
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
