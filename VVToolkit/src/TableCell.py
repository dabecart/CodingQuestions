from PyQt6.QtWidgets import QTableWidgetItem

class TableCell(QTableWidgetItem):
    def __init__(self, content, associatedItem=None):
        super().__init__(content)

        # Each table cell will have a reference to the item, that way is easier to access to it
        # using the default callbacks of PyQt6.
        self.associatedItem = associatedItem
