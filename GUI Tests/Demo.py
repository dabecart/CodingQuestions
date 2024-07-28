import sys
from PyQt6.QtWidgets import QApplication, QLabel
from PyQt6.QtGui import QPixmap, QImage, QPainter, QColor
from PyQt6.QtCore import Qt, QByteArray, QBuffer, QIODevice
from PyQt6.QtSvg import QSvgRenderer
import res_pack  # Import your compiled resources

def recolor_svg(icon_path, color):
    # Load the SVG data from the resource
    svg_data = None
    with open(icon_path, 'r') as file:
        svg_data = file.read()

    if svg_data is None:
        raise FileNotFoundError(f"SVG file not found: {icon_path}")

    # Modify the SVG data to change the fill color
    colored_svg_data = svg_data.replace('fill="#000000"', f'fill="{color}"')
    
    # Convert the modified SVG data to QByteArray
    byte_array = QByteArray()
    buffer = QBuffer(byte_array)
    buffer.open(QIODevice.OpenModeFlag.WriteOnly)
    buffer.write(colored_svg_data.encode('utf-8'))
    buffer.close()

    # Load the modified SVG data into QSvgRenderer
    renderer = QSvgRenderer(byte_array)
    
    # Create a QImage to render the SVG onto
    image = QImage(renderer.defaultSize(), QImage.Format.Format_ARGB32)
    image.fill(Qt.GlobalColor.transparent)  # Fill with transparency

    # Render the SVG onto the QImage
    painter = QPainter(image)
    renderer.render(painter)
    painter.end()

    # Convert QImage to QPixmap for display
    pixmap = QPixmap.fromImage(image)
    return pixmap

def main():
    app = QApplication(sys.argv)

    # Use the recolor function to get a white version of the SVG
    try:
        pixmap = recolor_svg(':/file-open.svg', "white")
        label = QLabel()
        label.setPixmap(pixmap)
        label.show()
        sys.exit(app.exec())
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
