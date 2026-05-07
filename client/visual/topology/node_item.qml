import QtQuick 2.15

Rectangle {
    id: nodeItem
    
    property string nodeType: "reader"
    property float load: 0.0
    property bool online: true
    
    width: 32
    height: 32
    radius: 16
    
    color: {
        if (!online) return "#666666"
        switch(nodeType) {
            case "gateway": return "#4CAF50"
            case "server": return "#2196F3"
            default: return "#FF9800"
        }
    }
    
    opacity: online ? 1.0 : 0.5
    
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: parent.height * 0.6
        radius: width / 2
        color: "#FFFFFF"
        opacity: 0.8
    }
    
    Text {
        anchors.centerIn: parent
        text: nodeType === "reader" ? "R" : 
              nodeType === "gateway" ? "G" : "S"
        color: "#333333"
        font.bold: true
        font.pixelSize: 12
    }
    
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 4
        color: "#E91E63"
        width: parent.width * Math.min(load, 1.0)
        radius: 2
        opacity: load > 0 ? 0.8 : 0
    }
    
    glow: load > 0.7 ? 1 : 0
    glowColor: "#FF5722"
}