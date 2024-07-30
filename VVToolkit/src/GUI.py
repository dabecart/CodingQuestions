from typing import Optional

from PyQt6.QtWidgets import (
    QMainWindow, QTableWidget, QTableWidgetItem, QCheckBox, QVBoxLayout, QWidget, QHeaderView,
    QLabel, QFormLayout, QSplitter, QHBoxLayout, QPushButton, QAbstractItemView, QMessageBox,
    QFileDialog
)
from PyQt6.QtCore import Qt, QEvent
from PyQt6.QtGui import QIntValidator, QIcon

from DataFields import Item, loadItemsFromFile, saveItemsToFile;
from LabeledEditLine import LabeledLineEdit
from Icons import createIcon

class ItemTable(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Verification and Validation Toolkit")
        self.setGeometry(100, 100, 800, 600)
        self.setWindowIcon(QIcon(':logo'))
        self.colorTheme = "dark"
        
        # Create the main splitter
        self.splitter = QSplitter(Qt.Orientation.Vertical)
        self.setCentralWidget(self.splitter)
        self.splitter.hide()

        # Menu Bar
        menubar = self.menuBar()

        file_menu = menubar.addMenu('&File')

        new_action = file_menu.addAction(createIcon(':file-new', self.colorTheme), '&New...')
        new_action.setShortcut("Ctrl+N")
        new_action.setStatusTip("Create a new file")
        new_action.triggered.connect(self.new_file)

        open_action = file_menu.addAction(createIcon(':file-open', self.colorTheme), '&Open...')
        open_action.setShortcut("Ctrl+O")
        open_action.setStatusTip("Open a file")
        open_action.triggered.connect(self.open_file)

        save_action = file_menu.addAction(createIcon(':file-save', self.colorTheme),'&Save')
        save_action.setShortcut("Ctrl+S")
        save_action.setStatusTip("Save the current file")
        save_action.triggered.connect(self.save_file)

        close_file_action = file_menu.addAction('&Close file')
        close_file_action.setShortcut("Ctrl+W")
        close_file_action.setStatusTip("Close the current file")
        close_file_action.triggered.connect(self.close_file)

        file_menu.addSeparator()

        close_action = file_menu.addAction(createIcon(':quit', self.colorTheme), '&Quit')
        close_action.setShortcut("Ctrl+Q")
        close_action.setStatusTip("Quit the application")
        close_action.triggered.connect(self.close)

        edit_menu = menubar.addMenu('&Edit')

        # Set up undo action
        self.undo_stack = []
        undo_action = edit_menu.addAction(createIcon(':edit-undo', self.colorTheme),'&Undo')
        undo_action.setShortcut("Ctrl+Z")
        undo_action.setStatusTip("Undo the last operation")
        undo_action.triggered.connect(self.undo)

        # Set up redo action
        self.redo_stack = []
        redo_action = edit_menu.addAction(createIcon(':edit-redo', self.colorTheme),'&Redo')
        redo_action.setShortcut("Ctrl+Y")
        redo_action.setStatusTip("Redo the last operation")
        redo_action.triggered.connect(self.redo)

        settings_menu = menubar.addMenu('&Settings')
        programsett_action = settings_menu.addAction(createIcon(':settings-program', self.colorTheme),'&Program settings')
        programsett_action.setShortcut("Ctrl+R")
        programsett_action.setStatusTip("Configure the program behavior.")

        help_menu = menubar.addMenu('&Help')
        about_action = help_menu.addAction(createIcon(':help-about', self.colorTheme), '&About')
        about_action.setShortcut("Ctrl+H")
        about_action.setStatusTip("Get help and info about this program.")

        # Tool bar
        fileToolBar = self.addToolBar('File')
        fileToolBar.addAction(new_action)
        fileToolBar.addAction(open_action)
        fileToolBar.addAction(save_action)

        editToolBar = self.addToolBar('Edit')
        editToolBar.addAction(undo_action)
        editToolBar.addAction(redo_action)

        settingsToolBar = self.addToolBar('Edit')
        settingsToolBar.addAction(programsett_action)

        # Field to store if the file is not saved.
        self.unsaved_changes = False
        # Field to save the currently opened file.
        self.current_file: Optional[str] = None

        # Bottom status bar
        self.statusBar = self.statusBar()
        self.statusBar.showMessage("Ready", 3000)
        self.statusBarPermanent = QLabel("")
        self.statusBar.addPermanentWidget(self.statusBarPermanent)

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
        
        # Set table properties
        self.table_widget.verticalHeader().setVisible(False)  # Remove row numbers
        self.table_widget.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)  # Select entire rows
        self.table_widget.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection) # Single row selection
        # Enable sorting
        self.table_widget.setSortingEnabled(True)
        
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

    def new_file(self):
        if self.unsaved_changes:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                self.save_file()

        self.items = []
        self.current_file = "Unnamed.json"
        self.populate_table()

        # Hide the details pane and show the window's content.
        self.details_widget.hide()
        self.splitter.show()

        self.statusBar.showMessage("New file created", 3000)

        self.unsaved_changes = True

    def open_file(self):
        if self.unsaved_changes:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                self.save_file()

        file_name, _ = QFileDialog.getOpenFileName(self, 'Open File', '', 'JSON Files (*.json)')
        if file_name:
            try:
                self.items = loadItemsFromFile(file_name)
                self.current_file = file_name
                self.populate_table()

                self.statusBarPermanent.setText(f"Current file: <b>{self.current_file}</b>")

                self.statusBar.showMessage("File opened", 3000)

                # Hide the details pane and show the window's content.
                self.details_widget.hide()
                self.splitter.show()
            except Exception as e:
                QMessageBox.critical(self, 'Error', f'Could not open file: {e}')

    def save_file(self):
        if not self.current_file:
            QMessageBox.warning(self, 'No File', 'No file selected. Please open a file first.')
            return False

        try:
            if self.current_file == "Unnamed.json":
                file_name, _ = QFileDialog.getSaveFileName(self, 'Save File', '', 'JSON Files (*.json)')
                if file_name:
                    self.current_file = file_name
                else:
                    return False

            saveItemsToFile(self.items, self.current_file)
            self.unsaved_changes = False
            self.statusBar.showMessage("File saved", 3000)
            return True
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Could not save file: {e}')
        return False

    def close_file(self):
        if self.unsaved_changes:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                if not self.save_file():
                    return

        # Hide the whole window pane.
        self.splitter.hide()
        self.current_file = None
        self.unsaved_changes = False

        self.statusBar.showMessage("File closed", 3000)

    def closeEvent(self, event):
        if self.unsaved_changes:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                event.ignore()
                return
            if reply == QMessageBox.StandardButton.Yes:
                if not self.save_file():
                    event.ignore()
                    return
        event.accept()

    def populate_table(self):
        self.table_widget.setRowCount(len(self.items))
        self.table_widget.setColumnCount(5)
        self.table_widget.setHorizontalHeaderLabels(["ID", "Name", "Category", "Repetitions", "Enabled"])

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

        # When opening, the cells get populated and think that a change has happened. Not the case.
        self.unsaved_changes = False

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
                if self.checkIDOk(inputID) == 0: 
                    item.id = int(self.table_widget.item(row, column).text())
                else:
                    # Restore the original value
                    self.table_widget.item(row, column).setText(str(item.id))
                    return
        
        elif column == 1:
            if not self.table_widget.item(row, column).text():
                self.table_widget.item(row, column).setText(item.name)
            else:
                item.name = self.table_widget.item(row, column).text()
        elif column == 2:
            if not self.table_widget.item(row, column).text():
                self.table_widget.item(row, column).setText(item.category)
            else:
                item.category = self.table_widget.item(row, column).text()
        elif column == 3:
            try:
                item.repetitions = int(self.table_widget.item(row, column).text())
            except ValueError:
                self.table_widget.item(row, column).setText(str(item.repetitions))
            return
        
        self.unsaved_changes = True
        
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
        
        try:
            item.repetitions = int(self.repetitions_field.text())
            self.repetitions_field.clearError()
        except ValueError:
            self.repetitions_field.setError("Repetitions must be a number.")
            return        
        
        item.enabled = self.enabled_field.isChecked()

        self.table_widget.item(self.current_row, 0).setText(str(item.id))
        self.table_widget.item(self.current_row, 1).setText(item.name)
        self.table_widget.item(self.current_row, 2).setText(item.category)
        self.table_widget.item(self.current_row, 3).setText(str(item.repetitions))
        self.table_widget.cellWidget(self.current_row, 4).blockSignals(True)
        self.table_widget.cellWidget(self.current_row, 4).setChecked(item.enabled)
        self.table_widget.cellWidget(self.current_row, 4).blockSignals(False)
        
        self.unsaved_changes = True

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
    def checkIDOk(self, newID) -> int:
        if type(newID) is str:
            try:
                newID = int(newID)
            except ValueError:
                return 1

        for i, item in enumerate(self.items):
            if i != self.current_row and item.id == newID:
                return 2
        return 0

    def validate_id(self):
        new_id = self.id_field.text()
        match self.checkIDOk(new_id):
            case 0: self.id_field.clearError()
            case 1: self.id_field.setError("This ID is not a number.")
            case 2: self.id_field.setError("This ID is already in use.")

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

        self.unsaved_changes = True

    def remove_item(self):
        selected_items = self.table_widget.selectedItems()
        if selected_items:
            row = selected_items[0].row()
            item = self.items.pop(row)
            self.table_widget.removeRow(row)
            self.undo_stack.append(('add', item))
            self.details_widget.hide()

            self.unsaved_changes = True

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