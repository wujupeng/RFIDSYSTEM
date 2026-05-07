import QtQuick 6.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 80
    color: "#0F1830"
    opacity: 0.95
    border.color: "#1E3A5F"
    border.width: 1
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 16
        
        // Playback Controls
        RowLayout {
            spacing: 8
            
            Button {
                id: playButton
                icon.source: isPlaying ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                onClicked: togglePlay()
                background: Rectangle {
                    color: "#1E3A5F"
                    radius: 8
                }
            }
            
            Button {
                icon.source: "qrc:/icons/rewind.svg"
                onClicked: rewind()
                background: Rectangle {
                    color: "#1E3A5F"
                    radius: 8
                }
            }
            
            Button {
                icon.source: "qrc:/icons/forward.svg"
                onClicked: forward()
                background: Rectangle {
                    color: "#1E3A5F"
                    radius: 8
                }
            }
        }
        
        // Speed Control
        RowLayout {
            spacing: 4
            
            Label {
                text: "Speed:"
                color: "#888888"
                font.pixelSize: 12
            }
            
            ButtonGroup {
                id: speedGroup
                
                Button {
                    text: "1x"
                    checkable: true
                    checked: playbackSpeed === 1
                    onClicked: setSpeed(1)
                    background: Rectangle {
                        color: speedGroup.checkedButton === parent ? "#00D2FF" : "#1E3A5F"
                        radius: 4
                    }
                }
                
                Button {
                    text: "2x"
                    checkable: true
                    checked: playbackSpeed === 2
                    onClicked: setSpeed(2)
                    background: Rectangle {
                        color: speedGroup.checkedButton === parent ? "#00D2FF" : "#1E3A5F"
                        radius: 4
                    }
                }
                
                Button {
                    text: "4x"
                    checkable: true
                    checked: playbackSpeed === 4
                    onClicked: setSpeed(4)
                    background: Rectangle {
                        color: speedGroup.checkedButton === parent ? "#00D2FF" : "#1E3A5F"
                        radius: 4
                    }
                }
                
                Button {
                    text: "16x"
                    checkable: true
                    checked: playbackSpeed === 16
                    onClicked: setSpeed(16)
                    background: Rectangle {
                        color: speedGroup.checkedButton === parent ? "#00D2FF" : "#1E3A5F"
                        radius: 4
                    }
                }
            }
        }
        
        // Timeline Slider
        Slider {
            id: timelineSlider
            Layout.fillWidth: true
            Layout.preferredWidth: 400
            from: 0
            to: 100
            value: currentProgress
            onValueChanged: seek(value)
            background: Rectangle {
                color: "#1E3A5F"
                height: 6
                radius: 3
            }
            handle: Rectangle {
                color: "#00D2FF"
                width: 16
                height: 16
                radius: 8
                border.color: "#FFFFFF"
                border.width: 2
            }
        }
        
        // Time Display
        Label {
            text: formatTime(currentTime) + " / " + formatTime(totalDuration)
            color: "#FFFFFF"
            font.pixelSize: 14
            font.monospace: true
        }
        
        // Live Indicator
        Rectangle {
            id: liveIndicator
            width: 8
            height: 8
            radius: 4
            color: isLive ? "#00FF88" : "#FF3B30"
            
            Behavior on color {
                ColorAnimation { duration: 300 }
            }
        }
        
        Label {
            text: isLive ? "LIVE" : "PLAYBACK"
            color: isLive ? "#00FF88" : "#FF3B30"
            font.pixelSize: 12
        }
        
        Button {
            text: "LIVE"
            onClicked: switchToLive()
            background: Rectangle {
                color: isLive ? "#00D2FF" : "#1E3A5F"
                radius: 4
            }
        }
    }
    
    // Properties
    property bool isPlaying: false
    property bool isLive: true
    property real playbackSpeed: 1
    property real currentProgress: 0
    property int currentTime: 0
    property int totalDuration: 3600000
    
    // Timer
    Timer {
        id: playbackTimer
        interval: 1000 / playbackSpeed
        running: isPlaying && !isLive
        repeat: true
        onTriggered: advanceTime()
    }
    
    // Functions
    function togglePlay() {
        if (isLive) {
            isLive = false
        }
        isPlaying = !isPlaying
    }
    
    function rewind() {
        currentProgress = Math.max(0, currentProgress - 10)
        currentTime = (currentProgress / 100) * totalDuration
    }
    
    function forward() {
        currentProgress = Math.min(100, currentProgress + 10)
        currentTime = (currentProgress / 100) * totalDuration
    }
    
    function seek(value) {
        currentProgress = value
        currentTime = (value / 100) * totalDuration
    }
    
    function advanceTime() {
        currentTime += 1000
        if (currentTime >= totalDuration) {
            currentTime = totalDuration
            isPlaying = false
        }
        currentProgress = (currentTime / totalDuration) * 100
    }
    
    function setSpeed(speed) {
        playbackSpeed = speed
        playbackTimer.interval = 1000 / speed
    }
    
    function switchToLive() {
        isLive = true
        isPlaying = false
        currentTime = Date.now()
    }
    
    function formatTime(ms) {
        var seconds = Math.floor(ms / 1000)
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor(minutes / 60)
        
        seconds = seconds % 60
        minutes = minutes % 60
        
        return String(hours).padStart(2, "0") + ":" + 
               String(minutes).padStart(2, "0") + ":" + 
               String(seconds).padStart(2, "0")
    }
}