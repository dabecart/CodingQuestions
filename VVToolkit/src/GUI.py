from PyQt6.QtWidgets import (
    QMainWindow, QTableWidget, QTableWidgetItem, QCheckBox, QVBoxLayout, QWidget, QHeaderView,
    QLabel, QFormLayout, QSplitter, QHBoxLayout, QPushButton, QMessageBox, QFileDialog
)
from PyQt6.QtCore import Qt, QEvent
from PyQt6.QtGui import QIntValidator, QIcon, QPalette

from typing import Optional
from copy import deepcopy

from DataFields import Item, loadItemsFromFile, saveItemsToFile;
from LabeledEditLine import LabeledLineEdit
from CodeTextField import CodeTextField
from SettingsWindow import ProgramConfig, SettingsWindow
from Icons import createIcon

class ItemTable(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Verification and Validation Toolkit")
        self.setGeometry(100, 100, 800, 600)
        self.setWindowIcon(QIcon(':logo'))

        # Stores the configuration of the program.
        self.config = ProgramConfig()

        # Check if the color is closer to black (dark mode) or white (light mode)
        color = self.palette().color(QPalette.ColorRole.Window)
        brightness = (color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) / 255
        self.config.colorTheme = "dark" if  brightness < 0.5 else "light"

        # Create the main splitter
        self.splitter = QSplitter(Qt.Orientation.Vertical)
        self.setCentralWidget(self.splitter)
        self.splitter.hide()

        # Menu Bar
        menubar = self.menuBar()

        fileMenu = menubar.addMenu('&File')

        newAction = fileMenu.addAction('&New...')
        newAction.setShortcut("Ctrl+N")
        newAction.setStatusTip("Create a new file")
        newAction.triggered.connect(self.newFile)

        openAction = fileMenu.addAction('&Open...')
        openAction.setShortcut("Ctrl+O")
        openAction.setStatusTip("Open a file")
        openAction.triggered.connect(self.openFile)

        saveAction = fileMenu.addAction('&Save')
        saveAction.setShortcut("Ctrl+S")
        saveAction.setStatusTip("Save the current file")
        saveAction.triggered.connect(self.saveFile)

        closeFileAction = fileMenu.addAction('&Close file')
        closeFileAction.setShortcut("Ctrl+W")
        closeFileAction.setStatusTip("Close the current file")
        closeFileAction.triggered.connect(self.closeFile)

        fileMenu.addSeparator()

        quitAction = fileMenu.addAction('&Quit')
        quitAction.setShortcut("Ctrl+Q")
        quitAction.setStatusTip("Quit the application")
        quitAction.triggered.connect(self.close)

        editMenu = menubar.addMenu('&Edit')

        # Set up undo action
        self.undoStack = []
        undoAction = editMenu.addAction('&Undo')
        undoAction.setShortcut("Ctrl+Z")
        undoAction.setStatusTip("Undo the last operation")
        undoAction.triggered.connect(self.undo)

        # Set up redo action
        self.redoStack = []
        redoAction = editMenu.addAction('&Redo')
        redoAction.setShortcut("Ctrl+Y")
        redoAction.setStatusTip("Redo the last operation")
        redoAction.triggered.connect(self.redo)

        editMenu.addSeparator()

        addItemAction = editMenu.addAction('&Add item')
        addItemAction.setShortcut("Alt+N")
        addItemAction.setStatusTip("Add an item to the list")
        addItemAction.triggered.connect(self.addItem)

        removeItemAction = editMenu.addAction('&Remove item')
        removeItemAction.setShortcut("Del")
        removeItemAction.setStatusTip("Remove an item from the list")
        removeItemAction.triggered.connect(self.removeItem)

        duplicateItemAction = editMenu.addAction('&Duplicate item')
        duplicateItemAction.setShortcut("Alt+D")
        duplicateItemAction.setStatusTip("Duplicate an item from the list")
        duplicateItemAction.triggered.connect(self.duplicateItem)

        settingsMenu = menubar.addMenu('&Settings')
        programSettAction = settingsMenu.addAction('&Program settings')
        programSettAction.setShortcut("Ctrl+R")
        programSettAction.setStatusTip("Configure the program behavior.")
        programSettAction.triggered.connect(self.changeConfig)

        helpMenu = menubar.addMenu('&Help')
        aboutAction = helpMenu.addAction('&About')
        aboutAction.setShortcut("Ctrl+H")
        aboutAction.setStatusTip("Get help and info about this program.")

        # Add icons to all actions.
        self.actionsIcons = [
            [newAction,             ':file-new'],
            [openAction,            ':file-open'],
            [saveAction,            ':file-save'],
            [quitAction,            ':quit'],
            [undoAction,            ':edit-undo'],
            [redoAction,            ':edit-redo'],
            [addItemAction,         ':item-add'],
            [removeItemAction,      ':item-remove'],    
            [duplicateItemAction,   ':item-duplicate'],        
            [programSettAction,     ':settings-program'],    
            [aboutAction,           ':help-about']
        ]
        self.redrawIcons(self.config)

        # Tool bar
        fileToolBar = self.addToolBar('File')
        fileToolBar.addAction(newAction)
        fileToolBar.addAction(openAction)
        fileToolBar.addAction(saveAction)

        editToolBar = self.addToolBar('Edit')
        editToolBar.addAction(undoAction)
        editToolBar.addAction(redoAction)

        settingsToolBar = self.addToolBar('Edit')
        settingsToolBar.addAction(programSettAction)

        # Field to store if the file is not saved.
        self.unsavedChanges = False
        # Field to save the currently opened file.
        self.currentFile: Optional[str] = None

        # Bottom status bar
        self.statusBar = self.statusBar()
        self.statusBar.showMessage("Ready", 3000)
        self.statusBarPermanent = QLabel("")
        self.statusBar.addPermanentWidget(self.statusBarPermanent)

        # Create a widget for the buttons
        buttonWidget = QWidget()
        buttonLayout = QHBoxLayout()
        buttonWidget.setLayout(buttonLayout)

        self.addButton = QPushButton(createIcon(':item-add', "green"), "Add Item")
        buttonLayout.addWidget(self.addButton)
        self.addButton.clicked.connect(self.addItem)

        self.removeButton = QPushButton(createIcon(':item-remove', "red"), "Remove Item")
        buttonLayout.addWidget(self.removeButton)
        self.removeButton.clicked.connect(self.removeItem)

        # Create the table widget
        self.tableWidget = QTableWidget()
        
        # Create a vertical layout for the table and buttons
        tableLayout = QVBoxLayout()
        tableLayout.addWidget(buttonWidget)
        tableLayout.addWidget(self.tableWidget)

        tableContainer = QWidget()
        tableContainer.setLayout(tableLayout)

        self.splitter.addWidget(tableContainer)
        
        # Set table properties
        self.tableWidget.verticalHeader().setVisible(False)  # Remove row numbers
        # self.tableWidget.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
        # self.tableWidget.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)

        # Enable sorting
        self.tableWidget.setSortingEnabled(True)
        
        # Connect cell click to show details
        self.currentRow = 0
        self.tableWidget.cellClicked.connect(self.showDetails)
        self.tableWidget.cellChanged.connect(self.updateDetailsFromTable)
        self.tableWidget.currentCellChanged.connect(self.updateDetailsFromSelection)
        self.tableWidget.viewport().installEventFilter(self)

        # Create the details widget with a header
        self.detailsWidget = QWidget()
        self.splitter.addWidget(self.detailsWidget)
        
        # Create a form layout for the details widget
        self.formLayout = QFormLayout()
        self.detailsWidget.setLayout(self.formLayout)

        # Add a header to the details widget
        header = QLabel("Item Details")
        header.setStyleSheet("font-weight: bold; margin-bottom: 10px;")
        self.formLayout.addRow(header)

        # Add fields to the form layout
        self.idField = LabeledLineEdit(validator=QIntValidator())
        self.idField.lineEdit.textChanged.connect(self.validateID)
        self.formLayout.addRow("ID:", self.idField)

        self.nameField = LabeledLineEdit()
        self.formLayout.addRow("Name:", self.nameField)

        self.categoryField = LabeledLineEdit()
        self.formLayout.addRow("Category:", self.categoryField)

        self.repetitionsField = LabeledLineEdit(validator=QIntValidator())
        self.formLayout.addRow("Repetitions:", self.repetitionsField)

        self.enabledField = QCheckBox()
        self.formLayout.addRow("Enabled:", self.enabledField)

        self.codeField = CodeTextField()
        self.formLayout.addRow("Command:", self.codeField)

        # Connect changes in the detail fields to update the table
        self.idField.lineEdit.textEdited.connect(self.updateTableFromDetails)
        self.nameField.lineEdit.textEdited.connect(self.updateTableFromDetails)
        self.categoryField.lineEdit.textEdited.connect(self.updateTableFromDetails)
        self.repetitionsField.lineEdit.textEdited.connect(self.updateTableFromDetails)
        self.enabledField.toggled.connect(self.updateTableFromDetails)
        self.codeField.textEdit.textChanged.connect(self.updateTableFromDetails)

        # Initially hide the details widget
        self.detailsWidget.hide()

    def redrawIcons(self, programConfig : ProgramConfig):
        for act in self.actionsIcons:
            act[0].setIcon(createIcon(act[1], programConfig))

    def newFile(self):
        if self.unsavedChanges:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                self.saveFile()

        self.items = []
        self.currentFile = "Unnamed.json"
        self.populateTable()

        # Hide the details pane and show the window's content.
        self.detailsWidget.hide()
        self.splitter.show()

        self.statusBar.showMessage("New file created", 3000)

        self.unsavedChanges = True

    def openFile(self):
        if self.unsavedChanges:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                self.saveFile()

        fileName, _ = QFileDialog.getOpenFileName(self, 'Open File', '', 'JSON Files (*.json)')
        if fileName:
            try:
                self.items = loadItemsFromFile(fileName)
                self.currentFile = fileName
                self.populateTable()

                self.statusBarPermanent.setText(f"Current file: <b>{self.currentFile}</b>")

                self.statusBar.showMessage("File opened", 3000)

                # Hide the details pane and show the window's content.
                self.detailsWidget.hide()
                self.splitter.show()
            except Exception as e:
                QMessageBox.critical(self, 'Error', f'Could not open file: {e}')

    def saveFile(self):
        if not self.currentFile:
            QMessageBox.warning(self, 'No File', 'No file selected. Please open a file first.')
            return False

        try:
            if self.currentFile == "Unnamed.json":
                fileName, _ = QFileDialog.getSaveFileName(self, 'Save File', '', 'JSON Files (*.json)')
                if fileName:
                    self.currentFile = fileName
                else:
                    return False

            saveItemsToFile(self.items, self.currentFile)
            self.unsavedChanges = False
            self.statusBar.showMessage("File saved", 3000)
            return True
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Could not save file: {e}')
        return False

    def closeFile(self):
        if self.unsavedChanges:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                return
            if reply == QMessageBox.StandardButton.Yes:
                if not self.saveFile():
                    return

        # Hide the whole window pane.
        self.splitter.hide()
        self.currentFile = None
        self.unsavedChanges = False

        self.statusBar.showMessage("File closed", 3000)

    def closeEvent(self, event):
        if self.unsavedChanges:
            reply = QMessageBox.question(self, 'Unsaved Changes',
                                         'You have unsaved changes. Do you want to save them?',
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No |
                                         QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes)
            if reply == QMessageBox.StandardButton.Cancel:
                event.ignore()
                return
            if reply == QMessageBox.StandardButton.Yes:
                if not self.saveFile():
                    event.ignore()
                    return
        event.accept()

    def populateTable(self):
        self.tableWidget.setRowCount(len(self.items))
        self.tableWidget.setColumnCount(5)
        self.tableWidget.setHorizontalHeaderLabels(["ID", "Name", "Category", "Repetitions", "Enabled"])

        for row, item in enumerate(self.items):
            self.tableWidget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.tableWidget.setItem(row, 1, QTableWidgetItem(item.name))
            self.tableWidget.setItem(row, 2, QTableWidgetItem(item.category))
            self.tableWidget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.updateEnabledCheckboxFromTable(r, state))
            self.tableWidget.setCellWidget(row, 4, checkbox)
        
        self.tableWidget.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        self.tableWidget.horizontalHeader().setSortIndicatorShown(True)

        # When opening, the cells get populated and think that a change has happened. Not the case.
        self.unsavedChanges = False

    def showDetails(self, row, column = -1):
        item = self.items[row]
        self.currentRow = row

        self.idField.setText(str(item.id))
        self.nameField.setText(item.name)
        self.categoryField.setText(item.category)
        self.repetitionsField.setText(str(item.repetitions))
        self.enabledField.setChecked(item.enabled)
        self.codeField.setText(item.runcode)

        self.detailsWidget.show()

        # Highlight the entire row
        self.tableWidget.selectRow(row)

        # Add warning labels for empty fields
        self.checkEmptyFields()

    def checkEmptyFields(self):
        self.idField.clearError()
        self.nameField.clearError()
        self.categoryField.clearError()
        self.repetitionsField.clearError()

        if not self.idField.text():
            self.idField.setError("ID cannot be empty.")
        if not self.nameField.text():
            self.nameField.setError("Name cannot be empty.")
        if not self.categoryField.text():
            self.categoryField.setError("Category cannot be empty.")
        if not self.repetitionsField.text():
            self.repetitionsField.setError("Repetitions cannot be empty.")

    def updateDetailsFromSelection(self, currentRow, currentColumn, previousRow, previousColumn):
        # Ensure a valid row is selected
        if currentRow != -1 and currentRow < len(self.items):  
            self.showDetails(currentRow, currentColumn)
        else:
            self.detailsWidget.hide()

    def updateDetailsFromTable(self, row, column):
        if row != self.currentRow:
            return
        item = self.items[row]
        if column == 0:
                inputID = int(self.tableWidget.item(row, column).text())
                if self.checkIDOk(inputID) == 0: 
                    item.id = int(self.tableWidget.item(row, column).text())
                else:
                    # Restore the original value
                    self.tableWidget.item(row, column).setText(str(item.id))
                    return
        
        elif column == 1:
            if not self.tableWidget.item(row, column).text():
                self.tableWidget.item(row, column).setText(item.name)
            else:
                item.name = self.tableWidget.item(row, column).text()
        elif column == 2:
            if not self.tableWidget.item(row, column).text():
                self.tableWidget.item(row, column).setText(item.category)
            else:
                item.category = self.tableWidget.item(row, column).text()
        elif column == 3:
            try:
                item.repetitions = int(self.tableWidget.item(row, column).text())
            except ValueError:
                self.tableWidget.item(row, column).setText(str(item.repetitions))
            return
        
        self.unsavedChanges = True
        
    def updateTableFromDetails(self):
        item = self.items[self.currentRow]
        try:
            item.id = int(self.idField.text())
            self.idField.clearError()
        except ValueError:
            self.idField.setError("ID must be a number.")
            return

        item.name = self.nameField.text()
        
        item.category = self.categoryField.text()
        
        try:
            item.repetitions = int(self.repetitionsField.text())
            self.repetitionsField.clearError()
        except ValueError:
            self.repetitionsField.setError("Repetitions must be a number.")
            return        
        
        item.enabled = self.enabledField.isChecked()

        item.runcode = self.codeField.getCommand(self.config.validateCommands)

        self.tableWidget.item(self.currentRow, 0).setText(str(item.id))
        self.tableWidget.item(self.currentRow, 1).setText(item.name)
        self.tableWidget.item(self.currentRow, 2).setText(item.category)
        self.tableWidget.item(self.currentRow, 3).setText(str(item.repetitions))
        self.tableWidget.cellWidget(self.currentRow, 4).blockSignals(True)
        self.tableWidget.cellWidget(self.currentRow, 4).setChecked(item.enabled)
        self.tableWidget.cellWidget(self.currentRow, 4).blockSignals(False)
        
        self.unsavedChanges = True

    def updateEnabledCheckboxFromTable(self, row, state):
        # Update the row when clicking on the checkbox.
        self.currentRow = row
        self.showDetails(row)

        item = self.items[row]
        item.enabled = (state == Qt.CheckState.Checked.value)
        self.enabledField.setChecked(item.enabled)

    # Check that the ID is not being used.
    def checkIDOk(self, newID) -> int:
        if type(newID) is str:
            try:
                newID = int(newID)
            except ValueError:
                return 1

        for i, item in enumerate(self.items):
            if i != self.currentRow and item.id == newID:
                return 2
        return 0

    def validateID(self):
        newID = self.idField.text()
        match self.checkIDOk(newID):
            case 0: self.idField.clearError()
            case 1: self.idField.setError("This ID is not a number.")
            case 2: self.idField.setError("This ID is already in use.")

    def deselectAll(self):
        self.tableWidget.clearSelection()
        self.detailsWidget.hide()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_Escape:
            self.deselectAll()
        super().keyPressEvent(event)

    def eventFilter(self, source, event):
        if event.type() == QEvent.Type.MouseButtonPress and source is self.tableWidget.viewport():
            if not self.tableWidget.indexAt(event.pos()).isValid():
                self.deselectAll()
        return super().eventFilter(source, event)

    def addItem(self, newItem):
        # Find the maximum current ID
        maxID = max(item.id for item in self.items) if self.items else 0
        if type(newItem) is bool:
            newItem=Item(maxID + 1, "", "", 0, True)
        elif type(newItem) is Item:
            newItem.id = maxID + 1
        else:
            raise Exception(f"Unexpected type received ({type(newItem)})")

        self.items.append(newItem)
        self.tableWidget.setRowCount(len(self.items))
        row = self.tableWidget.rowCount() - 1
        self.tableWidget.setItem(row, 0, QTableWidgetItem(str(newItem.id)))
        self.tableWidget.setItem(row, 1, QTableWidgetItem(newItem.name))
        self.tableWidget.setItem(row, 2, QTableWidgetItem(newItem.category))
        self.tableWidget.setItem(row, 3, QTableWidgetItem(str(newItem.repetitions)))
        checkbox = QCheckBox()
        checkbox.setChecked(newItem.enabled)
        checkbox.stateChanged.connect(lambda state, r=row: self.updateEnabledCheckboxFromTable(r, state))
        self.tableWidget.setCellWidget(row, 4, checkbox)
        self.undoStack.append(('remove', newItem))
        self.tableWidget.scrollToBottom()

        self.unsavedChanges = True

    def removeItem(self):
        selectedItem = self.tableWidget.selectedItems()
        if selectedItem:
            row = selectedItem[0].row()
            item = self.items.pop(row)
            self.tableWidget.removeRow(row)
            self.undoStack.append(('add', item))
            self.detailsWidget.hide()

            self.unsavedChanges = True

    def duplicateItem(self):
        selectedItem = self.tableWidget.selectedItems()
        if selectedItem:
            row = selectedItem[0].row()
            self.addItem(deepcopy(self.items[row]))
            self.unsavedChanges = True

    def changeConfig(self):
        settingsWindow = SettingsWindow(self.config, self)
        settingsWindow.exec()

    def undo(self):
        if not self.undoStack:
            return
        action, item = self.undoStack.pop()
        if action == 'add':
            self.items.append(item)
            self.redoStack.append(('remove', item))
            self.tableWidget.setRowCount(len(self.items))
            row = self.tableWidget.rowCount() - 1
            self.tableWidget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.tableWidget.setItem(row, 1, QTableWidgetItem(item.name))
            self.tableWidget.setItem(row, 2, QTableWidgetItem(item.category))
            self.tableWidget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.updateEnabledCheckboxFromTable(r, state))
            self.tableWidget.setCellWidget(row, 4, checkbox)
        elif action == 'remove':
            self.items.remove(item)
            self.redoStack.append(('add', item))
            for i in range(self.tableWidget.rowCount()):
                if int(self.tableWidget.item(i, 0).text()) == item.id:
                    self.tableWidget.removeRow(i)
                    break

    def redo(self):
        if not self.redoStack:
            return
        action, item = self.redoStack.pop()
        if action == 'add':
            self.items.append(item)
            self.undoStack.append(('remove', item))
            self.tableWidget.setRowCount(len(self.items))
            row = self.tableWidget.rowCount() - 1
            self.tableWidget.setItem(row, 0, QTableWidgetItem(str(item.id)))
            self.tableWidget.setItem(row, 1, QTableWidgetItem(item.name))
            self.tableWidget.setItem(row, 2, QTableWidgetItem(item.category))
            self.tableWidget.setItem(row, 3, QTableWidgetItem(str(item.repetitions)))
            checkbox = QCheckBox()
            checkbox.setChecked(item.enabled)
            checkbox.stateChanged.connect(lambda state, r=row: self.updateEnabledCheckboxFromTable(r, state))
            self.tableWidget.setCellWidget(row, 4, checkbox)
        elif action == 'remove':
            self.items.remove(item)
            self.undoStack.append(('add', item))
            for i in range(self.tableWidget.rowCount()):
                if int(self.tableWidget.item(i, 0).text()) == item.id:
                    self.tableWidget.removeRow(i)
                    break