#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace ground_truth {

struct TruthAnchor {
    uint64_t anchor_id;
    std::string name;
    float x;
    float y;
    float z;
    float uncertainty_m;
    bool active;
    uint64_t last_checked_timestamp;
};

class TruthAnchorManager {
public:
    static TruthAnchorManager& instance();
    
    void addAnchor(uint64_t anchor_id, const std::string& name, float x, float y, float z, float uncertainty_m);
    
    void removeAnchor(uint64_t anchor_id);
    
    void updateAnchor(uint64_t anchor_id, float x, float y, float z);
    
    const TruthAnchor* getAnchor(uint64_t anchor_id) const;
    
    std::vector<TruthAnchor> getActiveAnchors() const;
    
    size_t getAnchorCount() const;
    
    void loadAnchors(const std::vector<TruthAnchor>& anchors);
    
    std::vector<TruthAnchor> exportAnchors() const;
    
    bool isValidAnchor(uint64_t anchor_id) const;
    
private:
    TruthAnchorManager() = default;
    
    std::map<uint64_t, TruthAnchor> anchors_;
};

} // namespace ground_truth