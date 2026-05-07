import QtQuick 6.5

Canvas {
    id: root
    anchors.fill: parent
    
    property int gridSize: 50
    property color gridColor: "#1E3A5F"
    
    onPaint: {
        var ctx = getContext("2d")
        ctx.strokeStyle = gridColor
        ctx.lineWidth = 1
        
        // Draw vertical lines
        for (var x = 0; x <= width; x += gridSize) {
            ctx.beginPath()
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height)
            ctx.stroke()
        }
        
        // Draw horizontal lines
        for (var y = 0; y <= height; y += gridSize) {
            ctx.beginPath()
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
            ctx.stroke()
        }
        
        // Draw origin marker
        ctx.fillStyle = "#00D2FF"
        ctx.beginPath()
        ctx.arc(0, 0, 5, 0, Math.PI * 2)
        ctx.fill()
    }
}