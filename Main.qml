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
        }
}
