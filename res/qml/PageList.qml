import QtQuick 2.3
import QtQuick.Controls 2.5

Rectangle {

    property real sizeScale: 1.0

    id: pageList
    y: 40 * sizeScale
    width: 248 * sizeScale
    height: 38 * sizeScale + (packageModel.pageCount < 4 ? packageModel.pageCount : 4) * 118 * sizeScale
    radius: 8 * sizeScale
    color: "transparent"
    opacity: 0

    Behavior on opacity {
         PropertyAnimation { }
    }

    Behavior on y {
         PropertyAnimation { }
    }

    onVisibleChanged: {
        if (visible) {
            opacity = 1
            y = 0
        } else {
            opacity = 0
            y = 40 * sizeScale
        }
    }

    ListView {
        id: list
        x: 4 * sizeScale
        y: 64 * sizeScale
        spacing: 40 * sizeScale
        width: parent.width - 8 * sizeScale
        height: parent.height - 38 * sizeScale
        model: packageModel
        delegate: page
        contentY: 0
        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            policy: list.contentHeight > list.height + 10
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            visible: true
            focus: true
        }

        onVisibleChanged: {
            if (visible) {
                list.positionViewAtIndex(packageModel.currentIndex, ListView.Center)
                spacing = 0
                y = 24 * sizeScale
            } else {
                spacing = 40 * sizeScale
                y = 64 * sizeScale
            }
        }

        Behavior on spacing {
             PropertyAnimation { }
        }

        Behavior on y {
             PropertyAnimation { }
        }
    }

    Component {
        id: page
        Rectangle {
            width: 240 * sizeScale
            height: 118 * sizeScale
            color: "transparent"

            Rectangle {
                x: 4 * sizeScale
                y: 4 * sizeScale
                width: 24 * sizeScale
                height: 21 * sizeScale
                color: index == packageModel.currentIndex ? "#FF008FFF" : "transparent"
                radius: 4 * sizeScale
                Text {
                    anchors.centerIn: parent
                    color: index == packageModel.currentIndex ? "white" : "black"
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 16 * sizeScale
                    text: index + 1
                }
            }

            Rectangle {
                x: 32 * sizeScale
                y: 5 * sizeScale
                width: 192 * sizeScale
                height: 108 * sizeScale
                radius: 4 * sizeScale
                color: "#1B2526"

                Image {
                    anchors.fill: parent
                    id: thumbnail
                    cache: false
                    source: "image://resource/" + thumb

                    Rectangle {
                        anchors.fill: parent
                        radius: 4 * sizeScale
                        color: "transparent"
                        border.width: index == packageModel.currentIndex ? 3 : 1
                        border.color: index == packageModel.currentIndex ? "#FF008FFF" : "#FFE2E3E4"
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //packageModel.switchPage(index)
                    whiteCanvasTools.setOption("goto", index)
                }
            }
        }
    }
}
