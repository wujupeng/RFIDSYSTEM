import QtQuick 6.5

Item {
    id: root
    
    property var points: []
    property float colorHue: 0.0
    
    Canvas {
        anchors.fill: parent
        
        onPaint: {
            if (!points || points.length < 2) return
            
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            for (var i = 1; i < points.length; i++) {
                var p1 = points[i - 1]
                var p2 = points[i]
                
                var gradient = ctx.createLinearGradient(p1.x, p1.y, p2.x, p2.y)
                gradient.addColorStop(0, Qt.hsla(colorHue, 0.8, 0.6, p1.alpha))
                gradient.addColorStop(1, Qt.hsla(colorHue, 0.8, 0.6, p2.alpha))
                
                ctx.strokeStyle = gradient
                ctx.lineWidth = 3
                ctx.lineCap = "round"
                ctx.lineJoin = "round"
                
                ctx.beginPath()
                ctx.moveTo(p1.x, p1.y)
                ctx.lineTo(p2.x, p2.y)
                ctx.stroke()
                
                // Glow effect
                ctx.strokeStyle = Qt.hsla(colorHue, 0.8, 0.8, p1.alpha * 0.5)
                ctx.lineWidth = 8
                ctx.stroke()
            }
            
            // Draw end point glow
            if (points.length > 0) {
                var lastPoint = points[points.length - 1]
                var glowGradient = ctx.createRadialGradient(
                    lastPoint.x, lastPoint.y, 0,
                    lastPoint.x, lastPoint.y, 20
                )
                glowGradient.addColorStop(0, Qt.hsla(colorHue, 1.0, 0.8, lastPoint.alpha))
                glowGradient.addColorStop(1, Qt.hsla(colorHue, 1.0, 0.8, 0))
                
                ctx.fillStyle = glowGradient
                ctx.beginPath()
                ctx.arc(lastPoint.x, lastPoint.y, 20, 0, Math.PI * 2)
                ctx.fill()
            }
        }
    }
}