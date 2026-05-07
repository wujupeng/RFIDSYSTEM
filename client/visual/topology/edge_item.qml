import QtQuick 2.15

Canvas {
    id: edgeCanvas
    
    property float x1: 0
    property float y1: 0
    property float x2: 0
    property float y2: 0
    property float strength: 1.0
    property float flow: 0.0
    property bool active: true
    
    width: Math.abs(x2 - x1) + 50
    height: Math.abs(y2 - y1) + 50
    
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        
        var startX = x1 < x2 ? 25 : width - 25
        var startY = y1 < y2 ? 25 : height - 25
        var endX = x1 < x2 ? width - 25 : 25
        var endY = y1 < y2 ? height - 25 : 25
        
        ctx.beginPath()
        ctx.moveTo(startX, startY)
        ctx.lineTo(endX, endY)
        
        ctx.strokeStyle = active ? "#2196F3" : "#9E9E9E"
        ctx.lineWidth = 2 + strength * 4
        
        if (!active) {
            ctx.setLineDash([5, 5])
        }
        
        ctx.stroke()
        
        if (active && flow > 0.1) {
            var progress = (Date.now() / 200) % 1
            var px = startX + (endX - startX) * progress
            var py = startY + (endY - startY) * progress
            
            ctx.beginPath()
            ctx.arc(px, py, 4, 0, Math.PI * 2)
            ctx.fillStyle = "#FF5722"
            ctx.fill()
        }
    }
}