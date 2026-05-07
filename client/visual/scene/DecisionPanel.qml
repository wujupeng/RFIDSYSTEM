import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 320
    height: 420
    radius: 12
    color: "#1a1a2e"
    border.color: "#4a4a6a"
    border.width: 1
    visible: false
    
    property string assetName: ""
    property string action: ""
    property real confidence: 0.0
    property real missingRisk: 0.0
    property real abnormalRisk: 0.0
    property real idleRisk: 0.0
    property string reason: ""
    property var topKActions: []
    
    function show(x, y) {
        root.x = x - root.width / 2
        root.y = y - root.height / 2
        root.visible = true
    }
    
    function hide() {
        root.visible = false
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            
            ColumnLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Asset:"
                    color: "#888899"
                    font.pixelSize: 12
                }
                
                Text {
                    text: assetName
                    color: "#ffffff"
                    font.pixelSize: 18
                    font.bold: true
                }
            }
            
            Button {
                id: closeBtn
                width: 24
                height: 24
                background: Rectangle {
                    color: "#3a3a5a"
                    radius: 4
                }
                contentItem: Text {
                    text: "×"
                    color: "#ffffff"
                    font.pixelSize: 18
                }
                onClicked: root.hide()
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#3a3a5a"
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Action:"
                color: "#888899"
                font.pixelSize: 12
            }
            
            Text {
                text: action
                color: getActionColor(action)
                font.pixelSize: 20
                font.bold: true
            }
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Confidence:"
                color: "#888899"
                font.pixelSize: 12
            }
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: (confidence * 100).toFixed(0) + "%"
                    color: "#00d4ff"
                    font.pixelSize: 24
                    font.bold: true
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    Layout.height: 8
                    radius: 4
                    color: "#2a2a4a"
                    
                    Rectangle {
                        width: parent.width * confidence
                        height: parent.height
                        radius: 4
                        color: "#00d4ff"
                    }
                }
            }
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Risk Assessment:"
                color: "#888899"
                font.pixelSize: 12
                Layout.topMargin: 4
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                
                ColumnLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Missing"
                        color: "#ff6b6b"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: (missingRisk * 100).toFixed(0) + "%"
                        color: "#ff6b6b"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
                
                ColumnLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Abnormal"
                        color: "#ffd93d"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: (abnormalRisk * 100).toFixed(0) + "%"
                        color: "#ffd93d"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
                
                ColumnLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Idle"
                        color: "#6bcb77"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: (idleRisk * 100).toFixed(0) + "%"
                        color: "#6bcb77"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
            }
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Reason:"
                color: "#888899"
                font.pixelSize: 12
                Layout.topMargin: 4
            }
            
            Text {
                text: reason
                color: "#cccccc"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Top-K Actions:"
                color: "#888899"
                font.pixelSize: 12
                Layout.topMargin: 4
            }
            
            ListView {
                Layout.fillWidth: true
                Layout.height: 80
                model: topKActions
                
                delegate: RowLayout {
                    Layout.fillWidth: true
                    height: 24
                    
                    Text {
                        text: index + 1 + "."
                        color: "#888899"
                        font.pixelSize: 12
                        width: 24
                    }
                    
                    Text {
                        text: model.action
                        color: getActionColor(model.action)
                        font.pixelSize: 13
                        width: 100
                    }
                    
                    Text {
                        text: (model.score * 100).toFixed(0) + "%"
                        color: "#00d4ff"
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Layout.topMargin: 8
            
            Button {
                Layout.fillWidth: true
                text: "Confirm INSPECT"
                background: Rectangle {
                    color: "#00d4ff"
                    radius: 6
                }
                contentItem: Text {
                    text: "Confirm INSPECT"
                    color: "#1a1a2e"
                    font.pixelSize: 14
                    font.bold: true
                }
                onClicked: {
                    reportFeedback(assetName, "CONFIRM_INSPECT")
                    root.hide()
                }
            }
            
            Button {
                Layout.fillWidth: true
                text: "Ignore"
                background: Rectangle {
                    color: "#3a3a5a"
                    radius: 6
                }
                contentItem: Text {
                    text: "Ignore"
                    color: "#ffffff"
                    font.pixelSize: 14
                }
                onClicked: {
                    reportFeedback(assetName, "IGNORE")
                    root.hide()
                }
            }
            
            Button {
                Layout.fillWidth: true
                text: "Alert"
                background: Rectangle {
                    color: "#ffd93d"
                    radius: 6
                }
                contentItem: Text {
                    text: "Alert"
                    color: "#1a1a2e"
                    font.pixelSize: 14
                    font.bold: true
                }
                onClicked: {
                    reportFeedback(assetName, "CHANGE_TO_ALERT")
                    root.hide()
                }
            }
        }
    }
    
    function getActionColor(actionStr) {
        switch(actionStr) {
            case "INSPECT": return "#ff6b6b"
            case "ALERT": return "#ffd93d"
            case "REALLOCATE": return "#4dabf7"
            default: return "#888899"
        }
    }
    
    function reportFeedback(asset, action) {
        console.log("Feedback for " + asset + ": " + action)
    }
}