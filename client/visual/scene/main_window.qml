import QtQuick 6.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: mainWindow
    visible: true
    width: 1920
    height: 1080
    title: "RFID Spatial Intelligence Platform"
    color: "#0B1020"
    
    // Main Layout
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Left Panel - Navigation
        Rectangle {
            width: 200
            color: "#0F1830"
            border.color: "#1E3A5F"
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                
                // Logo
                RowLayout {
                    spacing: 8
                    Layout.topMargin: 16
                    
                    Image {
                        source: "qrc:/icons/logo.svg"
                        width: 32
                        height: 32
                    }
                    
                    Label {
                        text: "Spatial AI"
                        color: "#00D2FF"
                        font.bold: true
                        font.pixelSize: 16
                    }
                }
                
                // Navigation Items
                ColumnLayout {
                    spacing: 4
                    Layout.topMargin: 24
                    
                    NavButton {
                        text: "Dashboard"
                        icon: "qrc:/icons/dashboard.svg"
                        checked: currentView === "dashboard"
                        onClicked: switchView("dashboard")
                    }
                    
                    NavButton {
                        text: "Factory View"
                        icon: "qrc:/icons/factory.svg"
                        checked: currentView === "factory"
                        onClicked: switchView("factory")
                    }
                    
                    NavButton {
                        text: "Analytics"
                        icon: "qrc:/icons/analytics.svg"
                        checked: currentView === "analytics"
                        onClicked: switchView("analytics")
                    }
                    
                    NavButton {
                        text: "Settings"
                        icon: "qrc:/icons/settings.svg"
                        checked: currentView === "settings"
                        onClicked: switchView("settings")
                    }
                }
                
                // Status Indicators
                ColumnLayout {
                    spacing: 8
                    Layout.topMargin: 24
                    
                    Label {
                        text: "System Status"
                        color: "#888888"
                        font.pixelSize: 12
                    }
                    
                    StatusIndicator {
                        label: "Server"
                        status: "online"
                    }
                    
                    StatusIndicator {
                        label: "gRPC"
                        status: "online"
                    }
                    
                    StatusIndicator {
                        label: "Bandit"
                        status: "online"
                    }
                    
                    StatusIndicator {
                        label: "GPU"
                        status: "online"
                    }
                }
            }
        }
        
        // Main Content Area
        Rectangle {
            Layout.fillWidth: true
            color: "#0B1020"
            
            StackLayout {
                id: contentStack
                anchors.fill: parent
                
                // Dashboard View
                DashboardView {
                    id: dashboardView
                }
                
                // Factory View (Spatial Intelligence)
                Item {
                    anchors.fill: parent
                    
                    // Factory Scene
                    FactoryScene {
                        id: factoryScene
                        anchors.fill: parent
                        anchors.bottomMargin: 80
                    }
                    
                    // AI Overlay
                    AiOverlay {
                        id: aiOverlay
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: timeline.top
                        anchors.rightMargin: 16
                        anchors.topMargin: 16
                    }
                    
                    // Timeline
                    Timeline {
                        id: timeline
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                    }
                }
                
                // Analytics View
                AnalyticsView {
                    id: analyticsView
                }
                
                // Settings View
                SettingsView {
                    id: settingsView
                }
            }
        }
    }
    
    // Properties
    property string currentView: "factory"
    
    // Functions
    function switchView(view) {
        currentView = view
        contentStack.currentIndex = getViewIndex(view)
    }
    
    function getViewIndex(view) {
        switch(view) {
            case "dashboard": return 0
            case "factory": return 1
            case "analytics": return 2
            case "settings": return 3
            default: return 0
        }
    }
}