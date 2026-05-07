import QtQuick 6.5

Item {
    id: root
    width: radius * 2
    height: radius * 2
    x: xPos - radius
    y: yPos - radius
    
    property float xPos: 0
    property float yPos: 0
    property float radius: 50
    property float score: 0.0
    property string label: ""
    
    // Outer ring
    Rectangle {
        anchors.centerIn: parent
        width: radius * 2
        height: radius * 2
        radius: width / 2
        color: "transparent"
        border.color: ringColor
        border.width: 3
        opacity: 0.6
    }
    
    // Inner filled circle
    Rectangle {
        anchors.centerIn: parent
        width: radius * 1.5
        height: radius * 1.5
        radius: width / 2
        color: fillColor
        opacity: 0.4
        
        // Pulse animation
        property real pulseScale: 1
        scale: pulseScale
        
        Timer {
            interval: 1000
            running: score > 0.6
            repeat: true
            onTriggered: {
                pulseScale = 1 + Math.sin(Date.now() / 500) * 0.1
            }
        }
    }
    
    // Label
    Label {
        anchors.centerIn: parent
        text: label
        color: "#FFFFFF"
        font.pixelSize: 12
        font.bold: true
        opacity: 0.8
    }
    
    // Score indicator
    Label {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 4
        text: (score * 100).toFixed(0) + "%"
        color: textColor
        font.pixelSize: 10
        font.bold: true
    }
    
    function getRingColor() {
        if (score < 0.3) return "#00FF88"
        if (score < 0.6) return "#FFB020"
        if (score < 0.9) return "#FF8C00"
        return "#FF3B30"
    }
    
    function getFillColor() {
        if (score < 0.3) return "#00FF88"
        if (score < 0.6) return "#FFB020"
        if (score < 0.9) return "#FF8C00"
        return "#FF3B30"
    }
    
    function getTextColor() {
        if (score < 0.3) return "#00FF88"
        if (score < 0.6) return "#FFB020"
        return "#FF3B30"
    }
    
    property color ringColor: getRingColor()
    property color fillColor: getFillColor()
    property color textColor: getTextColor()
}