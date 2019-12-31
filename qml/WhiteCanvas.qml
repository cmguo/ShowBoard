import QtQuick 2.0

Rectangle {
    id: whitecanvas
    color: "transparent"
    children: {
        whiteCanvas
    }
    onWidthChanged: {
        whiteCanvas.width = width
    }
    onHeightChanged: {
        whiteCanvas.height = height;
    }
}
