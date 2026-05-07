import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: topologyView
    
    property var nodes: []
    property var edges: []
    
    Timer {
        interval: 100
        running: true
        repeat: true
        
        onTriggered: {
            for (var i = 0; i < edgesContainer.children.length; i++) {
                var edge = edgesContainer.children[i]
                edge.flow = Math.sin(Date.now() / 500 + i) * 0.5 + 0.5
            }
        }
    }
    
    Item {
        id: edgesContainer
        anchors.fill: parent
        z: 1
    }
    
    Item {
        id: nodesContainer
        anchors.fill: parent
        z: 2
    }
    
    function updateTopology(graph) {
        edgesContainer.children = []
        nodesContainer.children = []
        
        for (var i = 0; i < graph.nodes.length; i++) {
            var node = graph.nodes[i]
            var nodeItem = nodeComponent.createObject(nodesContainer)
            nodeItem.x = node.x * 10
            nodeItem.y = node.y * 10
            nodeItem.nodeType = "reader"
            nodeItem.load = node.load
            nodeItem.online = node.online
        }
        
        for (var j = 0; j < graph.edges.length; j++) {
            var edge = graph.edges[j]
            var fromNode = findNode(edge.from)
            var toNode = findNode(edge.to)
            
            if (fromNode && toNode) {
                var edgeItem = edgeComponent.createObject(edgesContainer)
                edgeItem.x1 = fromNode.x * 10 + 16
                edgeItem.y1 = fromNode.y * 10 + 16
                edgeItem.x2 = toNode.x * 10 + 16
                edgeItem.y2 = toNode.y * 10 + 16
                edgeItem.strength = edge.strength
            }
        }
    }
    
    function findNode(readerId) {
        for (var i = 0; i < nodes.length; i++) {
            if (nodes[i].reader_id === readerId) {
                return nodes[i]
            }
        }
        return null
    }
    
    Component {
        id: nodeComponent
        NodeItem {}
    }
    
    Component {
        id: edgeComponent
        EdgeItem {}
    }
}