import QtQuick 2.0

Rectangle {
    id: whitecanvas
    color: "transparent"
    children: {
        whiteCanvas
    }

    function updateGeometry() {
        whiteCanvas.updateState();
    }

    onWidthChanged: {
        whiteCanvas.width = width
    }
    onHeightChanged: {
        whiteCanvas.height = height;
    }
}
