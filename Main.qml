import QtQuick
import org.freedesktop.gstreamer.Qt6GLVideoItem

Window {
    visible: true
    width: 1280
    height: 480
    x: 30
    y: 30
    color: "black"
    
    Item {
        anchors.fill: parent
        
        // Left camera
        GstGLQt6VideoItem {
            id: videoLeft
            objectName: "videoItemLeft"
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2
        }
        
        // Right camera  
        GstGLQt6VideoItem {
            id: videoRight
            objectName: "videoItemRight"
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2
        }
        
        // Separator line
        Rectangle {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
                bottom: parent.bottom
            }
            width: 2
            color: "#333333"
        }
    }
}