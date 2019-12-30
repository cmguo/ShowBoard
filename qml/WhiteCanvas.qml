import QtQuick 2.0

Rectangle {
    id: whitecanvas
    color: "transparent"
    children: {
        whiteCanvas
    }
    onParentChanged: {
        whiteCanvas.x = 0;
        whiteCanvas.y = 0;
        whiteCanvas.width = width;
        whiteCanvas.height = height;
    }
}
