import QtQuick 2.3
import QtQuick.Controls 2.5

Rectangle {

    property bool drag: false
    property real lastScrollPosY: 0;

    width: 248
    height: packageModel.pageCount < 4 ? 392 : 510
    radius: 8
    color: "#FFF9F9F9"

    ListView {
        id: pageList
        x: 4
        y: 4
        width: parent.width - 8
        height: parent.height - 8
        model: packageModel
        delegate: page
        contentY: 0
        ScrollBar.vertical: ScrollBar {
            id:scrollbar
            policy: pageList.contentHeight > pageList.height + 10
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            visible: true
            focus: true
            onPressedChanged: {
                drag = pressed;
                if (!pressed)
                    lastScrollPosY = scrollbar.position
            }
        }
        spacing: 20

        onMovementEnded: {
            lastScrollPosY = scrollbar.position
            drag = false
        }
        onMovementStarted: {
            drag = true;
        }

        onContentYChanged: {
            var changed = contentHeight>0&&height>0&&contentHeight>height;
            if(!drag && changed){
                if(contentHeight*(1-lastScrollPosY)<=height){
                    lastScrollPosY = (contentHeight-height)/contentHeight
                }
                scrollbar.position = lastScrollPosY
            }
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
                onClicked: {
                    packageModel.switchPage(index)
                }
            }
        }
    }
}
