#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ReaderNode {
    uint64_t reader_id;
    double x;
    double y;
    double coverage_radius;
    bool online;
    int antenna_count;
};

struct ReaderEdge {
    uint64_t from;
    uint64_t to;
    double signal_strength;
    double distance;
};

class ReaderGraph {
public:
    void addNode(const ReaderNode& node);
    
    void addEdge(uint64_t from, uint64_t to, double signal_strength);
    
    ReaderNode getNode(uint64_t reader_id);
    
    std::vector<ReaderEdge> getEdges(uint64_t reader_id);
    
    std::vector<ReaderNode> getOnlineReaders();
    
    void updateNodeStatus(uint64_t reader_id, bool online);
    
    size_t getNodeCount() const;
    
private:
    std::vector<ReaderNode> nodes_;
    std::vector<ReaderEdge> edges_;
};