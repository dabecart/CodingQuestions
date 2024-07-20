import sys
from dataclasses import dataclass
from typing import List
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTableWidget, QTableWidgetItem, QCheckBox, QVBoxLayout, QWidget, QHeaderView,
    QLineEdit, QLabel, QFormLayout, QSplitter, QFrame, QHBoxLayout, QPushButton, QAbstractItemView
)
from PyQt6.QtCore import Qt, QEvent
from PyQt6.QtGui import QIntValidator, QColor, QKeySequence, QPalette, QAction

@dataclass
class Item:
    id: int
    name: str
    category: str
    repetitions: int
    enabled: bool

class LabeledLineEdit(QWidget):
    def __init__(self, label_text="", parent=None, validator=None):
        super().__init__(parent)
        
        self.layout = QHBoxLayout()
        self.layout.setContentsMargins(0, 0, 0, 0)
        
        self.lineEdit = QLineEdit(self)
        if validator is not None:
            self.lineEdit.setValidator(validator)

        self.errorLabel = QLabel("", self)
        self.errorLabel.setStyleSheet("color: red; margin: 0px;")
        
        self.layout.addWidget(self.lineEdit)
        self.layout.addWidget(self.errorLabel)
        self.setLayout(self.layout)

        self.errorLabel.hide()  # Hide error label initially

    def setError(self, error_text):
        self.lineEdit.setStyleSheet("background-color: red;")
        self.errorLabel.setText(error_text)
        self.errorLabel.show()

    def clearError(self):
        self.lineEdit.setStyleSheet("")
        self.errorLabel.hide()

    def text(self):
        return self.lineEdit.text()

    def setText(self, text):
        self.lineEdit.setText(text)

class ItemTable(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Item List")
        self.setGeometry(100, 100, 800, 600)
        
        # Create the main splitter
        self.splitter = QSplitter(Qt.Orientation.Vertical)
        self.setCentralWidget(self.splitter)

        # Create a widget for the buttons
        button_widget = QWidget()
        button_layout = QHBoxLayout()
        button_widget.setLayout(button_layout)

        self.add_button = QPushButton("Add Item")
        self.remove_button = QPushButton("Remove Item")
        button_layout.addWidget(self.add_button)
        button_layout.addWidget(self.remove_button)

        self.add_button.clicked.connect(self.add_item)
        self.remove_button.clicked.connect(self.remove_item)

        # Create the table widget
        self.table_widget = QTableWidget()
        
        # Create a vertical layout for the table and buttons
        table_layout = QVBoxLayout()
        table_layout.addWidget(button_widget)
        table_layout.addWidget(self.table_widget)

        table_container = QWidget()
        table_container.setLayout(table_layout)

        self.splitter.addWidget(table_container)
        
        # Set table dimensions
        self.table_widget.setColumnCount(5)
        self.table_widget.setHorizontalHeaderLabels(["ID", "Name", "Category", "Repetitions", "Enabled"])
        self.table_widget.verticalHeader().setVisible(False)  # Remove row numbers
        self.table_widget.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)  # Select entire rows
        self.table_widget.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection) # Single row selection
        
        # Enable sorting
        self.table_widget.setSortingEnabled(True)
        
        # Populate the table with some example data
        self.populate_table()

        # Connect cell click to show details
        self.current_row = 0
        self.table_widget.cellClicked.connect(self.show_details)
        self.table_widget.cellChanged.connect(self.update_details_from_table)
        self.table_widget.currentCellChanged.connect(self.update_details_from_selection)
        self.table_widget.viewport().installEventFilter(self)

        # Create the details widget with a header
        self.details_widget = QWidget()
        self.splitter.addWidget(self.details_widget)
        
        # Create a form layout for the details widget
        self.form_layout = QFormLayout()
        self.details_widget.setLayout(self.form_layout)

        # Add a header to the details widget
        header = QLabel("Item Details")
        header.setStyleSheet("font-weight: bold; margin-bottom: 10px;")
        self.form_layout.addRow(header)

        # Add fields to the form layout
        self.id_field = LabeledLineEdit(validator=QIntValidator())
        self.id_field.lineEdit.textChanged.connect(self.validate_id)
        self.form_layout.addRow("ID:", self.id_field)

        self.name_field = LabeledLineEdit()
        self.form_layout.addRow("Name:", self.name_field)

        self.category_field = LabeledLineEdit()
        self.form_layout.addRow("Category:", self.category_field)

        self.repetitions_field = LabeledLineEdit(validator=QIntValidator())
        self.form_layout.addRow("Repetitions:", self.repetitions_field)

        self.enabled_field = QCheckBox()
        self.form_layout.addRow("Enabled:", self.enabled_field)

        # Connect changes in the detail fields to update the table
        self.id_field.lineEdit.textEdited.connect(self.update_table_from_details)
        self.name_field.lineEdit.textEdited.connect(self.update_table_from_details)
        self.category_field.lineEdit.textEdited.connect(self.update_table_from_details)
        self.repetitions_field.lineEdit.textEdited.connect(self.update_table_from_details)
        self.enabled_field.toggled.connect(self.update_table_from_details)

        # Initially hide the details widget
        self.details_widget.hide()

        # Set up undo action
        self.undo_stack = []
        undo_action = QAction("Undo", self)
        undo_action.setShortcut(QKeySequence("Ctrl+Z"))
        undo_action.triggered.connect(self.undo)
        self.addAction(undo_action)

        self.redo_stack = []

        redo_action = QAction("Redo", self)
        redo_action.setShortcut(QKeySequence("Ctrl+Y"))
        redo_action.triggered.connect(self.redo)
        self.addAction(redo_action)

    def populate_table(self):
        self.items: List[Item] = [
            Item(1, "Item 1", "A", 5, True),
            Item(2, "Item 2", "B", 2, False),
            Item(3, "Item 3", "A", 7, True),
            Item(4, "Item 4", "C", 1, False),
            Item(5, "Item 5", "B", 4, True),
        ]
        
        self.table_widget.setRowCount(len(self.items))
        
        for row, item in enumerate(self.items):
            self.table_widget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.table_widget.setItem(row, 1, QTableWidgetItem(item.name))
            self.table_widget.setItem(row, 2, QTableWidgetItem(item.category))
            self.table_widget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.update_enabled_from_table(r, state))
            self.table_widget.setCellWidget(row, 4, checkbox)
        
        self.table_widget.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        self.table_widget.horizontalHeader().setSortIndicatorShown(True)

    def show_details(self, row, column = -1):
        item = self.items[row]
        self.current_row = row
        self.id_field.setText(str(item.id))
        self.name_field.setText(item.name)
        self.category_field.setText(item.category)
        self.repetitions_field.setText(str(item.repetitions))
        self.enabled_field.blockSignals(True)
        self.enabled_field.setChecked(item.enabled)
        self.enabled_field.blockSignals(False)
        self.details_widget.show()

        # Highlight the entire row
        self.table_widget.selectRow(row)

        # Add warning labels for empty fields
        self.check_empty_fields()

    def check_empty_fields(self):
        self.id_field.clearError()
        self.name_field.clearError()
        self.category_field.clearError()
        self.repetitions_field.clearError()

        if not self.id_field.text():
            self.id_field.setError("ID cannot be empty.")
        if not self.name_field.text():
            self.name_field.setError("Name cannot be empty.")
        if not self.category_field.text():
            self.category_field.setError("Category cannot be empty.")
        if not self.repetitions_field.text():
            self.repetitions_field.setError("Repetitions cannot be empty.")

    def update_details_from_selection(self, current_row, current_column, previous_row, previous_column):
        if current_row != -1:  # Ensure a valid row is selected
            self.show_details(current_row, current_column)
        else:
            self.details_widget.hide()

    def update_details_from_table(self, row, column):
        if row != self.current_row:
            return
        item = self.items[row]
        if column == 0:
                inputID = int(self.table_widget.item(row, column).text())
                if self.checkIDOk(inputID): 
                    item.id = int(self.table_widget.item(row, column).text())
                else:
                    self.table_widget.item(row, column).setText(str(item.id))  # Restore original value
                    return
        
        elif column == 1:
            if not self.table_widget.item(row, column).text():
                self.table_widget.item(row, column).setText(item.name)  # Restore original value
            else:
                item.name = self.table_widget.item(row, column).text()
        elif column == 2:
            if not self.table_widget.item(row, column).text():
                self.table_widget.item(row, column).setText(item.category)  # Restore original value
            else:
                item.category = self.table_widget.item(row, column).text()
        elif column == 3:
            try:
                item.repetitions = int(self.table_widget.item(row, column).text())
            except ValueError:
                self.table_widget.item(row, column).setText(str(item.repetitions))  # Restore original value
            return
        
    def update_table_from_details(self):
        item = self.items[self.current_row]
        try:
            item.id = int(self.id_field.text())
            self.id_field.clearError()
        except ValueError:
            self.id_field.setError("ID must be a number.")
            return

        item.name = self.name_field.text()
        item.category = self.category_field.text()
        item.repetitions = int(self.repetitions_field.text())
        item.enabled = self.enabled_field.isChecked()

        self.table_widget.item(self.current_row, 0).setText(str(item.id))
        self.table_widget.item(self.current_row, 1).setText(item.name)
        self.table_widget.item(self.current_row, 2).setText(item.category)
        self.table_widget.item(self.current_row, 3).setText(str(item.repetitions))
        self.table_widget.cellWidget(self.current_row, 4).blockSignals(True)
        self.table_widget.cellWidget(self.current_row, 4).setChecked(item.enabled)
        self.table_widget.cellWidget(self.current_row, 4).blockSignals(False)

    def update_enabled_from_table(self, row, state):
        # Update the row when clicking on the checkbox.
        self.current_row = row
        self.show_details(row)

        item = self.items[row]
        item.enabled = (state == Qt.CheckState.Checked.value)
        self.enabled_field.blockSignals(True)
        self.enabled_field.setChecked(item.enabled)
        self.enabled_field.blockSignals(False)

    # Check that the ID is not being used.
    def checkIDOk(self, newID) -> bool:
        if type(newID) is str:
            try:
                newID = int(newID)
            except ValueError:
                return False

        for i, item in enumerate(self.items):
            if i != self.current_row and item.id == newID:
                return False
        return True

    def validate_id(self):
        new_id = self.id_field.text()
        if self.checkIDOk(new_id):
            self.id_field.clearError()
        else:
            self.id_field.setError("This ID is already in use.")

    def deselect_all(self):
        self.table_widget.clearSelection()
        self.details_widget.hide()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_Escape:
            self.deselect_all()
        super().keyPressEvent(event)

    def eventFilter(self, source, event):
        if event.type() == QEvent.Type.MouseButtonPress and source is self.table_widget.viewport():
            if not self.table_widget.indexAt(event.pos()).isValid():
                self.deselect_all()
        return super().eventFilter(source, event)

    def add_item(self):
        # Find the maximum current ID
        max_id = max(item.id for item in self.items) if self.items else 0
        new_item = Item(max_id + 1, "", "", 0, True)
        self.items.append(new_item)
        self.table_widget.setRowCount(len(self.items))
        row = self.table_widget.rowCount() - 1
        self.table_widget.setItem(row, 0, QTableWidgetItem(str(new_item.id)))
        self.table_widget.setItem(row, 1, QTableWidgetItem(new_item.name))
        self.table_widget.setItem(row, 2, QTableWidgetItem(new_item.category))
        self.table_widget.setItem(row, 3, QTableWidgetItem(str(new_item.repetitions)))
        checkbox = QCheckBox()
        checkbox.setChecked(new_item.enabled)
        checkbox.stateChanged.connect(lambda state, r=row: self.update_enabled_from_table(r, state))
        self.table_widget.setCellWidget(row, 4, checkbox)
        self.undo_stack.append(('remove', new_item))
        self.table_widget.scrollToBottom()

    def remove_item(self):
        selected_items = self.table_widget.selectedItems()
        if selected_items:
            row = selected_items[0].row()
            item = self.items.pop(row)
            self.table_widget.removeRow(row)
            self.undo_stack.append(('add', item))
            self.details_widget.hide()

    def undo(self):
        if not self.undo_stack:
            return
        action, item = self.undo_stack.pop()
        if action == 'add':
            self.items.append(item)
            self.redo_stack.append(('remove', item))
            self.table_widget.setRowCount(len(self.items))
            row = self.table_widget.rowCount() - 1
            self.table_widget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.table_widget.setItem(row, 1, QTableWidgetItem(item.name))
            self.table_widget.setItem(row, 2, QTableWidgetItem(item.category))
            self.table_widget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.update_enabled_from_table(r, state))
            self.table_widget.setCellWidget(row, 4, checkbox)
        elif action == 'remove':
            self.items.remove(item)
            self.redo_stack.append(('add', item))
            for i in range(self.table_widget.rowCount()):
                if int(self.table_widget.item(i, 0).text()) == item.id:
                    self.table_widget.removeRow(i)
                    break

    def redo(self):
        if not self.redo_stack:
            return
        action, item = self.redo_stack.pop()
        if action == 'add':
            self.items.append(item)
            self.undo_stack.append(('remove', item))
            self.table_widget.setRowCount(len(self.items))
            row = self.table_widget.rowCount() - 1
            self.table_widget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.table_widget.setItem(row, 1, QTableWidgetItem(item.name))
            self.table_widget.setItem(row, 2, QTableWidgetItem(item.category))
            self.table_widget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.update_enabled_from_table(r, state))
            self.table_widget.setCellWidget(row, 4, checkbox)
        elif action == 'remove':
            self.items.remove(item)
            self.undo_stack.append(('add', item))
            for i in range(self.table_widget.rowCount()):
                if int(self.table_widget.item(i, 0).text()) == item.id:
                    self.table_widget.removeRow(i)
                    break

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ItemTable()
    window.show()
    sys.exit(app.exec())
