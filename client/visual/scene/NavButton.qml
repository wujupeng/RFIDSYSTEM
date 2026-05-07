import QtQuick 6.5
import QtQuick.Controls 2.15

Button {
    id: root
    Layout.fillWidth: true
    height: 48
    checkable: true
    
    background: Rectangle {
        color: root.checked ? "#1E3A5F" : "transparent"
        radius: 8
        
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            color: root.checked ? "#00D2FF" : "transparent"
            radius: 2
        }
    }
    
    contentItem: RowLayout {
        spacing: 12
        
        Image {
            source: root.icon
            width: 20
            height: 20
            fillMode: Image.PreserveAspectFit
            color: root.checked ? "#00D2FF" : "#888888"
        }
        
        Label {
            text: root.text
            color: root.checked ? "#FFFFFF" : "#888888"
            font.pixelSize: 14
        }
    }
    
    property string icon: ""
}