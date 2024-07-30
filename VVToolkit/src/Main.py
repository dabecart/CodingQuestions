import sys
from GUI import ItemTable
from PyQt6.QtWidgets import QApplication
from PyQt6.QtGui import QIcon

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ItemTable()
    window.show()
    sys.exit(app.exec())