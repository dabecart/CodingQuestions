from PyQt6.QtWidgets import QApplication, QWidget, QFormLayout, QLineEdit, QLabel

class LineEditWithError(QWidget):
    def __init__(self, label, parent_layout, parent=None):
        super().__init__(parent)
        self.parent_layout = parent_layout
        self.error_label = None

        # Create the line edit
        self.line_edit = QLineEdit()
        
        # Add the line edit to the parent layout
        self.row = self.parent_layout.rowCount()
        self.parent_layout.addRow(label, self.line_edit)

    def setError(self, error_message):
        if not self.error_label:
            self.error_label = QLabel()
            self.error_label.setStyleSheet("color: red;")
            # Insert a new row for the error label right after the line edit's row
            self.parent_layout.insertRow(self.row + 1, "", self.error_label)
        self.error_label.setText(error_message)
        self.error_label.show()
        self.update_rows(1)  # Adjust row indices for subsequent LineEditWithError instances

    def clearError(self):
        if self.error_label:
            self.parent_layout.removeRow(self.row + 1)
            self.error_label = None
            self.update_rows(-1)  # Adjust row indices for subsequent LineEditWithError instances

    def update_rows(self, adjustment):
        # Adjust the row indices for all subsequent LineEditWithError instances
        for i in range(self.row + 1, self.parent_layout.rowCount()):
            widget = self.parent_layout.itemAt(i, QFormLayout.ItemRole.FieldRole).widget()
            if isinstance(widget, LineEditWithError):
                widget.row += adjustment

class FormExample(QWidget):
    def __init__(self):
        super().__init__()

        self.form_layout = QFormLayout()
        self.setLayout(self.form_layout)

        self.line_edits_with_error = []
        for i in range(3):  # Example with three line edits
            line_edit_with_error = LineEditWithError(f"Label {i + 1}:", self.form_layout)
            line_edit_with_error.line_edit.textChanged.connect(self.validate_input)
            self.line_edits_with_error.append(line_edit_with_error)

    def validate_input(self):
        for line_edit_with_error in self.line_edits_with_error:
            text = line_edit_with_error.line_edit.text()
            if self.is_input_valid(text):
                line_edit_with_error.clearError()
            else:
                line_edit_with_error.setError("Error: Input must be numeric")

    def is_input_valid(self, text):
        return text.isnumeric()

if __name__ == '__main__':
    app = QApplication([])
    form_example = FormExample()
    form_example.show()
    app.exec()
