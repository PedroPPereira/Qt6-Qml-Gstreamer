import QtQuick
import org.freedesktop.gstreamer.Qt6GLVideoItem
Window {
    visible: true
        width: 640
        height: 480
        x: 30
        y: 30
        color: "black"
    Item {
            anchors.fill: parent

            GstGLQt6VideoItem {
                id: video
                objectName: "videoItem"
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
            }

            Rectangle {
                color: Qt.rgba(0.3, 0.7, 0.5, 0.7)
                border.width: 1
                border.color: "#339999"
                anchors.centerIn: parent
                width : parent.width - 100
                height: 50
                radius: 8

                Text{
                    text: "Demo for Gstreamer"
                    anchors.fill: parent
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: Qt.AlignHCenter
                    font.pointSize: 24
                }

            }
        }
}
