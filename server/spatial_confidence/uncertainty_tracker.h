#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <map>

struct UncertaintyBounds {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    double std_dev;
};

class UncertaintyTracker {
public:
    static UncertaintyTracker& instance();
    
    void update(const std::string& epc, const TagPosition& pos);
    
    UncertaintyBounds getBounds(const std::string& epc);
    
    double getUncertainty(const std::string& epc);
    
    void reset(const std::string& epc);
    
private:
    UncertaintyTracker();
    
    std::map<std::string, std::vector<TagPosition>> history_;
};