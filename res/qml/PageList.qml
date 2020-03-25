import QtQuick 2.3
import QtQuick.Controls 2.5

Rectangle {

    id: pageList
    y: 40
    width: 248
    height: packageModel.pageCount < 4 ? 392 : 510
    radius: 8
    color: "#FFF9F9F9"
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
            y = 40
        }
    }

    ListView {
        id: list
        x: 4
        y: 44
        spacing: 40
        width: parent.width - 8
        height: parent.height - 8
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
                y = 4
            } else {
                spacing = 40
                y = 44
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
            width: 240
            height: 118
            color: "transparent"

            Rectangle {
                x: 4
                y: 4
                width: 24
                height: 21
                color: index == packageModel.currentIndex ? "#FF008FFF" : "transparent"
                radius: 4
                Text {
                    anchors.centerIn: parent
                    font.family: "MicrosoftYaHei"
                    font.pixelSize: 16
                    text: index + 1
                }
            }

            Image {
                id: thumbnail
                x: 32
                y: 5
                width: 192
                height: 108
                cache: false
                source: "image://resource/" + thumb

                Rectangle {
                    anchors.fill: parent
                    radius: 4
                    color: "transparent"
                    border.width: index == packageModel.currentIndex ? 2 : 0
                    border.color: "#FF008FFF"
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
