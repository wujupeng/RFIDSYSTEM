#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <map>

struct ConfidenceSource {
    std::string name;
    double weight;
    double confidence;
    bool trusted;
};

class ConfidenceEngine {
public:
    static ConfidenceEngine& instance();
    
    void updateSource(const std::string& name, double confidence, bool trusted);
    
    double getOverallConfidence();
    
    void propagateConfidence();
    
    void decay();
    
    std::vector<ConfidenceSource> getSources();
    
private:
    ConfidenceEngine();
    
    std::map<std::string, ConfidenceSource> sources_;
};