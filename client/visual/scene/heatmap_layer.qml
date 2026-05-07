import QtQuick 6.5
import QtQuick.Shapes 6.5

Rectangle {
    id: root
    color: "transparent"
    
    property var heatmapData: []
    
    ShaderEffectSource {
        id: heatmapSource
        anchors.fill: parent
        
        ShaderEffect {
            id: heatmapShader
            width: root.width
            height: root.height
            
            property variant heatmap: heatmapTexture
            property real maxValue: 1.0
            
            fragmentShader: "
                uniform lowp sampler2D heatmap;
                uniform highp float maxValue;
                varying highp vec2 qt_TexCoord0;
                
                void main() {
                    highp float heat = texture2D(heatmap, qt_TexCoord0).r / maxValue;
                    highp vec3 color;
                    
                    if (heat < 0.2) {
                        color = mix(vec3(0.0, 0.13, 0.38), vec3(0.0, 0.83, 1.0), heat * 5.0);
                    } else if (heat < 0.5) {
                        color = mix(vec3(0.0, 0.83, 1.0), vec3(1.0, 1.0, 0.0), (heat - 0.2) * 3.33);
                    } else if (heat < 0.8) {
                        color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.69, 0.13), (heat - 0.5) * 3.33);
                    } else {
                        color = mix(vec3(1.0, 0.69, 0.13), vec3(1.0, 0.23, 0.19), (heat - 0.8) * 5.0);
                    }
                    
                    gl_FragColor = vec4(color, heat * 0.6);
                }
            "
        }
    }
    
    function updateHeatmap(data) {
        heatmapData = data
        updateTexture()
    }
    
    function updateTexture() {
        if (!heatmapData.length) return
        
        var size = Math.sqrt(heatmapData.length)
        var image = Qt.createImage(size, size, QImage.Format_R32F)
        
        for (var i = 0; i < heatmapData.length; i++) {
            var cell = heatmapData[i]
            var idx = cell.y * size + cell.x
            image.setPixel(cell.x, cell.y, cell.value)
        }
        
        heatmapTexture = image
    }
}