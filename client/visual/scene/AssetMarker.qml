import QtQuick 6.5

Item {
    id: root
    width: size * 2
    height: size * 2
    x: xPos - size
    y: yPos - size
    
    property float xPos: 0
    property float yPos: 0
    property float risk: 0.0
    property int state: 0
    property float size: 8
    property string topAction: ""
    property double topScore: 0.0
    
    signal clicked()
    
    // Outer glow
    Rectangle {
        anchors.centerIn: parent
        width: size * 3
        height: size * 3
        radius: width / 2
        color: markerColor
        opacity: 0.3
        scale: 1 + Math.sin(Date.now() / 500) * 0.2
        
        Behavior on scale {
            NumberAnimation { duration: 500 }
        }
    }
    
    // Main marker
    Rectangle {
        anchors.centerIn: parent
        width: size * 2
        height: size * 2
        radius: width / 2
        color: markerColor
        border.color: "#FFFFFF"
        border.width: 1
        opacity: 1
        
        // Pulse animation for high risk
        property real pulseScale: 1
        scale: pulseScale
        
        Timer {
            interval: 200
            running: state === 2 || state === 3
            repeat: true
            onTriggered: {
                pulseScale = 1 + Math.sin(Date.now() / 200) * 0.3
            }
        }
    }
    
    // Action indicator
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: -4
        anchors.rightMargin: -4
        width: 8
        height: 8
        radius: 4
        color: actionColor
        
        visible: topScore > 0.5
    }
    
    // Click area
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
    
    function getMarkerColor() {
        switch(state) {
            case 2: return "#FFB020"  // INSPECT
            case 3: return "#FF3B30"  // SECURITY_ALERT
            default: {
                if (risk > 0.8) return "#FF3B30"
                if (risk > 0.5) return "#FFB020"
                return "#00D2FF"
            }
        }
    }
    
    function getActionColor() {
        switch(topAction) {
            case "INSPECT": return "#FFB020"
            case "ALERT": return "#FF3B30"
            case "NO_ACTION": return "#00FF88"
            default: return "#888888"
        }
    }
    
    property color markerColor: getMarkerColor()
    property color actionColor: getActionColor()
}