import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 320
    height: 200
    radius: 8
    color: "#1a1a2e"
    border.color: "#3a3a5a"
    border.width: 1
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: 16
    
    property real fps: 0
    property real frameTime: 0
    property real gpuTime: 0
    property real vramUsed: 0
    property real vramTotal: 4096
    property int droppedFrames: 0
    property real replayLag: 0
    property real bandwidth: 0
    property int subscribers: 0
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6
        
        Text {
            text: "System Health"
            color: "#00d4ff"
            font.pixelSize: 14
            font.bold: true
            Layout.fillWidth: true
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            ColumnLayout {
                Layout.fillWidth: true
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "FPS:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: fps.toFixed(1)
                        color: fps > 25 ? "#00ff88" : (fps > 15 ? "#ffd93d" : "#ff6b6b")
                        font.pixelSize: 14
                        font.bold: true
                    }
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Frame:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: frameTime.toFixed(1) + "ms"
                        color: frameTime < 50 ? "#00ff88" : (frameTime < 100 ? "#ffd93d" : "#ff6b6b")
                        font.pixelSize: 14
                    }
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "GPU:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: gpuTime.toFixed(1) + "ms"
                        color: gpuTime < 30 ? "#00ff88" : (gpuTime < 100 ? "#ffd93d" : "#ff6b6b")
                        font.pixelSize: 14
                    }
                }
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "VRAM:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: vramUsed.toFixed(0) + "MB"
                        color: vramUsed / vramTotal < 0.7 ? "#00ff88" : (vramUsed / vramTotal < 0.9 ? "#ffd93d" : "#ff6b6b")
                        font.pixelSize: 14
                    }
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Dropped:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: droppedFrames.toString()
                        color: droppedFrames > 10 ? "#ff6b6b" : "#00ff88"
                        font.pixelSize: 14
                    }
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Replay:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: replayLag.toFixed(0) + "ms"
                        color: replayLag < 100 ? "#00ff88" : (replayLag < 500 ? "#ffd93d" : "#ff6b6b")
                        font.pixelSize: 14
                    }
                }
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#3a3a5a"
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            ColumnLayout {
                Layout.fillWidth: true
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "BW:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: (bandwidth / 1024).toFixed(1) + " MB/s"
                        color: "#00d4ff"
                        font.pixelSize: 14
                    }
                }
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Subs:"
                        color: "#888899"
                        font.pixelSize: 12
                        width: 60
                    }
                    
                    Text {
                        text: subscribers.toString()
                        color: "#00ff88"
                        font.pixelSize: 14
                    }
                }
            }
        }
    }
}