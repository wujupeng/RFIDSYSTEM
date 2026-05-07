import QtQuick 6.5
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent
    
    // Scene dimensions (640m x 640m)
    property int sceneWidth: 640
    property int sceneHeight: 640
    property int gridSize: 64
    
    // Scale factor for rendering
    property real scale: Math.min(width / sceneWidth, height / sceneHeight)
    
    // Frame data
    property var assets: []
    property var heatmap: []
    property var congestion: []
    
    // View offset
    property real viewX: 0
    property real viewY: 0
    
    // Canvas for rendering
    Canvas {
        id: canvas
        anchors.fill: parent
        
        onPaint: {
            var ctx = getContext("2d")
            
            // Clear
            ctx.fillStyle = "#0B1020"
            ctx.fillRect(0, 0, width, height)
            
            // Draw grid
            drawGrid(ctx)
            
            // Draw heatmap
            drawHeatmap(ctx)
            
            // Draw congestion zones
            drawCongestion(ctx)
            
            // Draw assets
            drawAssets(ctx)
        }
    }
    
    function drawGrid(ctx) {
        ctx.strokeStyle = "#1E3A5F"
        ctx.lineWidth = 1
        
        var cellWidth = width / gridSize
        var cellHeight = height / gridSize
        
        for (var x = 0; x <= width; x += cellWidth) {
            ctx.beginPath()
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height)
            ctx.stroke()
        }
        
        for (var y = 0; y <= height; y += cellHeight) {
            ctx.beginPath()
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
            ctx.stroke()
        }
    }
    
    function drawHeatmap(ctx) {
        if (!heatmap || heatmap.length === 0) return
        
        var cellWidth = width / gridSize
        var cellHeight = height / gridSize
        
        for (var i = 0; i < heatmap.length; i++) {
            var cell = heatmap[i]
            var x = cell.x * cellWidth
            var y = cell.y * cellHeight
            
            // Color based on value
            var value = cell.value
            var color = getHeatColor(value)
            
            ctx.fillStyle = color
            ctx.fillRect(x, y, cellWidth, cellHeight)
        }
    }
    
    function getHeatColor(value) {
        // Blue -> Cyan -> Yellow -> Red
        if (value < 5) {
            var alpha = value / 5 * 0.5
            return "rgba(0, 200, 255, " + alpha + ")"
        } else if (value < 15) {
            var t = (value - 5) / 10
            return "rgba(" + Math.round(255 * t) + ", " + Math.round(200 + 55 * t) + ", 255, 0.6)"
        } else if (value < 25) {
            var t = (value - 15) / 10
            return "rgba(255, " + Math.round(255 - 95 * t) + ", 0, 0.7)"
        } else {
            return "rgba(255, 0, 0, 0.8)"
        }
    }
    
    function drawCongestion(ctx) {
        if (!congestion || congestion.length === 0) return
        
        for (var i = 0; i < congestion.length; i++) {
            var zone = congestion[i]
            
            // Convert scene coordinates to screen
            var x = zone.x * (width / sceneWidth)
            var y = zone.y * (height / sceneHeight)
            var radius = zone.radius * (width / sceneWidth)
            
            // Draw circle
            ctx.beginPath()
            ctx.arc(x, y, radius, 0, Math.PI * 2)
            
            // Color based on level
            if (zone.level === "CONGESTED") {
                ctx.fillStyle = "rgba(255, 59, 48, 0.4)"
                ctx.strokeStyle = "#FF3B30"
            } else {
                ctx.fillStyle = "rgba(255, 176, 32, 0.3)"
                ctx.strokeStyle = "#FFB020"
            }
            
            ctx.fill()
            ctx.lineWidth = 2
            ctx.stroke()
            
            // Draw label
            ctx.fillStyle = "#FFFFFF"
            ctx.font = "bold 10px Arial"
            ctx.textAlign = "center"
            ctx.fillText(zone.asset_count.toString(), x, y + radius + 15)
        }
    }
    
    function drawAssets(ctx) {
        if (!assets || assets.length === 0) return
        
        for (var i = 0; i < assets.length; i++) {
            var asset = assets[i]
            
            // Convert scene coordinates to screen
            var x = asset.x * (width / sceneWidth)
            var y = asset.y * (height / sceneHeight)
            var size = asset.size * scale
            
            // Draw asset point
            ctx.beginPath()
            ctx.arc(x, y, size, 0, Math.PI * 2)
            
            // Color based on state (risk)
            var color = getAssetColor(asset.state)
            ctx.fillStyle = color
            ctx.fill()
            
            // Draw glow effect
            ctx.shadowColor = color
            ctx.shadowBlur = 10
            ctx.fill()
            ctx.shadowBlur = 0
        }
    }
    
    function getAssetColor(state) {
        // 0: NORMAL (blue), 1: OBSERVE (yellow), 2: INSPECT (red)
        switch (state) {
            case 0: return "#00D2FF"
            case 1: return "#FFB020"
            case 2: return "#FF3B30"
            default: return "#00D2FF"
        }
    }
    
    // Update frame data
    function updateFrame(frame) {
        assets = frame.assets || []
        heatmap = frame.heatmap || []
        congestion = frame.congestion || []
        
        canvas.requestPaint()
    }
    
    // Test data generator
    function generateTestFrame() {
        var frame = {
            assets: [],
            heatmap: [],
            congestion: []
        }
        
        // Generate 100-200 assets
        var assetCount = 100 + Math.floor(Math.random() * 100)
        for (var i = 0; i < assetCount; i++) {
            frame.assets.push({
                x: Math.random() * sceneWidth,
                y: Math.random() * sceneHeight,
                state: Math.random() > 0.8 ? 2 : (Math.random() > 0.5 ? 1 : 0),
                size: 8
            })
        }
        
        // Generate heatmap
        for (var x = 0; x < gridSize; x++) {
            for (var y = 0; y < gridSize; y++) {
                var value = Math.floor(Math.random() * 30)
                if (value > 0) {
                    frame.heatmap.push({
                        x: x,
                        y: y,
                        value: value
                    })
                }
            }
        }
        
        // Generate congestion zones
        var zoneCount = Math.floor(Math.random() * 5)
        for (var z = 0; z < zoneCount; z++) {
            frame.congestion.push({
                x: Math.random() * sceneWidth,
                y: Math.random() * sceneHeight,
                radius: 30 + Math.random() * 20,
                asset_count: 20 + Math.floor(Math.random() * 20),
                level: Math.random() > 0.5 ? "CONGESTED" : "HIGH_DENSITY"
            })
        }
        
        updateFrame(frame)
    }
    
    // Auto update for testing
    Timer {
        interval: 100
        running: true
        repeat: true
        onTriggered: generateTestFrame()
    }
}