import sys
from PyQt6.QtWidgets import (QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QTextEdit, QComboBox, QLineEdit, QSizePolicy, QScrollArea)
from PyQt6.QtGui import QIcon, QPalette, QColor
from PyQt6.QtCore import QPropertyAnimation, QAbstractAnimation

class CollapsibleBox(QWidget):
    def __init__(self, icon_path, number, label_text, parent=None):
        super().__init__(parent)
        self.setSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.header = QWidget()
        self.header.setObjectName('header')
        self.header_layout = QHBoxLayout(self.header)

        self.arrow_label = QLabel("\u25B6")
        self.arrow_label.setFixedWidth(15)
        self.icon_label = QLabel()
        self.icon_label.setPixmap(QIcon(icon_path).pixmap(24, 24))
        self.number_label = QLabel(str(number))
        self.text_label = QLabel(label_text)

        self.header_layout.addWidget(self.arrow_label)
        self.header_layout.addWidget(self.icon_label)
        self.header_layout.addWidget(self.number_label)
        self.header_layout.addWidget(self.text_label)
        self.header_layout.addStretch()

        self.header.mousePressEvent = self.toggle_content

        self.content = QWidget()
        self.content_layout = QVBoxLayout(self.content)

        self.input_label = QLabel("Input:")
        self.input_text = QTextEdit()
        self.input_text.setReadOnly(True)
        self.output_label = QLabel("Output:")
        self.output_text = QTextEdit()
        self.output_text.setReadOnly(True)

        self.checking_mode_label = QLabel("Checking Mode:")
        self.checking_mode_combo = QComboBox()
        self.checking_mode_combo.addItems(["Equal output", "Conditional output"])
        self.checking_mode_combo.currentTextChanged.connect(self.on_checking_mode_changed)

        self.operator_combo = QComboBox()
        self.operator_combo.addItems(["==", "<>", "<", ">", "<=", ">="])
        self.operator_combo.setVisible(False)

        self.value_edit = QLineEdit()
        self.value_edit.setVisible(False)

        self.content_layout.addWidget(self.input_label)
        self.content_layout.addWidget(self.input_text)
        self.content_layout.addWidget(self.output_label)
        self.content_layout.addWidget(self.output_text)
        self.content_layout.addWidget(self.checking_mode_label)
        self.content_layout.addWidget(self.checking_mode_combo)
        self.content_layout.addWidget(self.operator_combo)
        self.content_layout.addWidget(self.value_edit)

        self.content.setVisible(False)

        self.mainWidget = QWidget()
        self.mainWidget.setObjectName('mainName')
        self.main_layout = QVBoxLayout()
        self.main_layout.addWidget(self.header)
        self.main_layout.addWidget(self.content)
        self.main_layout.addStretch()

        self.mainWidget.setLayout(self.main_layout)
        self.closedHeight = self.mainWidget.sizeHint().height()
        self.mainWidget.setMaximumHeight(self.closedHeight)

        self.selfLayout = QVBoxLayout()
        self.selfLayout.addWidget(self.mainWidget)
        self.setLayout(self.selfLayout)

        self.animation = QPropertyAnimation(self.content, b"maximumHeight")
        self.animation.setDuration(250)
        self.animation.finished.connect(self.on_animation_finished)

        self.setStyleSheet("""
            #header {
                background-color: #f0f0f0;
                border: 1px solid #ccc;
                padding: 2px;
                border-radius: 4px;
            }
            #mainName {
                background-color: #e0e0e0;
                border: 1px solid #ccc;
                padding: 2px;
                border-radius: 4px;
            }
            QLabel {
                font-size: 14px;
            }
        """)
        
    def toggle_content(self, event):
        if self.animation.state() == QAbstractAnimation.State.Running:
            return
        
        if self.content.isVisible():
            # Close the window.
            self.arrow_label.setText("\u25B6")
            self.animation.setStartValue(self.content.sizeHint().height())
            self.animation.setEndValue(0)
            self.animation.start()
        else:
            # Open the window.
            self.arrow_label.setText("\u25BC")
            self.mainWidget.setMaximumHeight(16777215) # Maximum allowed height.
            self.content.setVisible(True)
            self.animation.setStartValue(0)
            self.animation.setEndValue(self.content.sizeHint().height())
            self.animation.start()

    def on_animation_finished(self):
        if self.animation.endValue() == 0:
            self.content.setVisible(False)
            self.mainWidget.setMaximumHeight(self.closedHeight)

    def on_checking_mode_changed(self, text):
        if text == "Conditional output":
            self.operator_combo.setVisible(True)
            self.value_edit.setVisible(True)
        else:
            self.operator_combo.setVisible(False)
            self.value_edit.setVisible(False)


class MainWindow(QWidget):

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Collapsible Box Example")
        self.setGeometry(100, 100, 600, 400)  # Increase the width for the sidebar

        box1 = CollapsibleBox("icon.png", 1, "Label 1")
        box2 = CollapsibleBox("icon.png", 2, "Label 2")
        box3 = CollapsibleBox("icon.png", 3, "Label 3")

        sidebar_layout = QVBoxLayout()
        sidebar_layout.addWidget(box1)
        sidebar_layout.addWidget(box2)
        sidebar_layout.addWidget(box3)
        sidebar_layout.addStretch()

        sidebar = QWidget()
        sidebar.setLayout(sidebar_layout)

        scroll_area = QScrollArea()
        scroll_area.setWidget(sidebar)
        scroll_area.setWidgetResizable(True)

        main_layout = QVBoxLayout(self)
        main_layout.addWidget(scroll_area)

        self.setLayout(main_layout)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_win = MainWindow()
    main_win.show()
    sys.exit(app.exec())
