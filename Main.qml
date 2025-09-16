import QtQuick
import org.freedesktop.gstreamer.Qt6GLVideoItem

Window {
    visible: true
    width: 1280
    height: 960
    x: 30
    y: 30
    color: "black"
    
    Grid {
        anchors.fill: parent
        anchors.margins: 4
        rows: 2
        columns: 2
        spacing: 4
        
        GstGLQt6VideoItem {
            objectName: "videoItem0"
            width: (parent.width - parent.spacing) / parent.columns
            height: (parent.height - parent.spacing) / parent.rows
        }
        
        GstGLQt6VideoItem {
            objectName: "videoItem1"
            width: (parent.width - parent.spacing) / parent.columns
            height: (parent.height - parent.spacing) / parent.rows
        }
        
        GstGLQt6VideoItem {
            objectName: "videoItem2"
            width: (parent.width - parent.spacing) / parent.columns
            height: (parent.height - parent.spacing) / parent.rows
        }
        
        GstGLQt6VideoItem {
            objectName: "videoItem3"
            width: (parent.width - parent.spacing) / parent.columns
            height: (parent.height - parent.spacing) / parent.rows
        }
    }
}