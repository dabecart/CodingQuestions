class SignalBlocker:
    def __init__(self, *widgets):
        self.widgets = widgets

    def __enter__(self):
        for widget in self.widgets:
            widget.blockSignals(True)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for widget in self.widgets:
            widget.blockSignals(False)
