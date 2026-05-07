import QtQuick 6.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 320
    color: "#0F1830"
    opacity: 0.95
    radius: 12
    border.color: "#1E3A5F"
    border.width: 1
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            
            Image {
                source: "qrc:/icons/ai-icon.svg"
                width: 24
                height: 24
                fillMode: Image.PreserveAspectFit
            }
            
            Label {
                text: "AI Decision Panel"
                color: "#00D2FF"
                font.bold: true
                font.pixelSize: 16
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }
        
        // Selected Asset
        Rectangle {
            id: assetInfo
            Layout.fillWidth: true
            color: "#162040"
            radius: 8
            padding: 12
            
            ColumnLayout {
                spacing: 8
                
                Label {
                    text: "Selected Asset: " + (selectedAsset ? selectedAsset.name : "None")
                    color: "#FFFFFF"
                    font.pixelSize: 14
                }
                
                RowLayout {
                    spacing: 16
                    
                    ColumnLayout {
                        Label { text: "Risk:"; color: "#888888"; font.pixelSize: 12 }
                        Label { 
                            text: selectedAsset ? selectedAsset.risk.toFixed(2) : "0.00"
                            color: getRiskColor(selectedAsset ? selectedAsset.risk : 0)
                            font.bold: true
                            font.pixelSize: 16
                        }
                    }
                    
                    ColumnLayout {
                        Label { text: "State:"; color: "#888888"; font.pixelSize: 12 }
                        Label { 
                            text: getStateText(selectedAsset ? selectedAsset.state : 0)
                            color: getStateColor(selectedAsset ? selectedAsset.state : 0)
                            font.bold: true
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }
        
        // Bandit Actions
        Rectangle {
            Layout.fillWidth: true
            color: "#162040"
            radius: 8
            padding: 12
            
            ColumnLayout {
                spacing: 8
                
                Label {
                    text: "Bandit Top-K Actions"
                    color: "#00D2FF"
                    font.pixelSize: 14
                    font.bold: true
                }
                
                ColumnLayout {
                    id: actionList
                    spacing: 4
                    
                    Repeater {
                        model: banditActions
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: model.type
                                color: "#FFFFFF"
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignLeft
                            }
                            
                            Rectangle {
                                height: 12
                                Layout.fillWidth: true
                                Layout.maximumWidth: 150
                                color: "#1E3A5F"
                                radius: 6
                                
                                Rectangle {
                                    height: parent.height
                                    width: model.score * parent.width
                                    color: getActionColor(model.type)
                                    radius: 6
                                }
                            }
                            
                            Label {
                                text: (model.score * 100).toFixed(0) + "%"
                                color: "#FFFFFF"
                                font.pixelSize: 12
                                Layout.preferredWidth: 40
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }
        }
        
        // Risk Factors
        Rectangle {
            Layout.fillWidth: true
            color: "#162040"
            radius: 8
            padding: 12
            
            ColumnLayout {
                spacing: 8
                
                Label {
                    text: "Risk Factors"
                    color: "#FFB020"
                    font.pixelSize: 14
                    font.bold: true
                }
                
                ColumnLayout {
                    id: riskList
                    spacing: 4
                    
                    Repeater {
                        model: riskFactors
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: model.name
                                color: "#FFFFFF"
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignLeft
                            }
                            
                            Label {
                                text: model.score.toFixed(2)
                                color: model.score > 0.7 ? "#FF3B30" : "#FFFFFF"
                                font.pixelSize: 12
                                Layout.preferredWidth: 50
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }
        }
        
        // Bandit Stats
        Rectangle {
            Layout.fillWidth: true
            color: "#162040"
            radius: 8
            padding: 12
            
            ColumnLayout {
                spacing: 8
                
                Label {
                    text: "Bandit System Stats"
                    color: "#00D2FF"
                    font.pixelSize: 14
                    font.bold: true
                }
                
                GridLayout {
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 4
                    
                    Label { text: "Exploration:"; color: "#888888"; font.pixelSize: 12 }
                    Label { text: (banditStats.explorationRate * 100).toFixed(1) + "%"; color: "#FFFFFF"; font.pixelSize: 12 }
                    
                    Label { text: "Total Decisions:"; color: "#888888"; font.pixelSize: 12 }
                    Label { text: banditStats.totalDecisions; color: "#FFFFFF"; font.pixelSize: 12 }
                    
                    Label { text: "Avg Confidence:"; color: "#888888"; font.pixelSize: 12 }
                    Label { text: (banditStats.avgConfidence * 100).toFixed(0) + "%"; color: "#00FF88"; font.pixelSize: 12 }
                }
            }
        }
    }
    
    // Properties
    property var selectedAsset: null
    property var banditActions: []
    property var riskFactors: []
    property var banditStats: {
        explorationRate: 0.05,
        totalDecisions: 0,
        avgConfidence: 0.0
    }
    
    // Functions
    function selectAsset(assetId) {
        selectedAsset = assetModel.getAsset(assetId)
        if (selectedAsset) {
            banditActions = selectedAsset.actionScores
            riskFactors = selectedAsset.riskFactors || []
        }
    }
    
    function getRiskColor(risk) {
        if (risk > 0.8) return "#FF3B30"
        if (risk > 0.5) return "#FFB020"
        return "#00D2FF"
    }
    
    function getStateText(state) {
        switch(state) {
            case 0: return "NORMAL"
            case 1: return "OBSERVE"
            case 2: return "INSPECT"
            case 3: return "SECURITY ALERT"
            default: return "UNKNOWN"
        }
    }
    
    function getStateColor(state) {
        switch(state) {
            case 0: return "#00D2FF"
            case 1: return "#00FF88"
            case 2: return "#FFB020"
            case 3: return "#FF3B30"
            default: return "#888888"
        }
    }
    
    function getActionColor(action) {
        switch(action) {
            case "INSPECT": return "#FFB020"
            case "ALERT": return "#FF3B30"
            case "NO_ACTION": return "#00FF88"
            case "REALLOCATE": return "#9D4EDD"
            default: return "#888888"
        }
    }
}