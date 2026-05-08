#include "truth_anchor_manager.h"

namespace ground_truth {

TruthAnchorManager& TruthAnchorManager::instance() {
    static TruthAnchorManager manager;
    return manager;
}

void TruthAnchorManager::addAnchor(uint64_t anchor_id, const std::string& name, 
                                   float x, float y, float z, float uncertainty_m) {
    TruthAnchor anchor;
    anchor.anchor_id = anchor_id;
    anchor.name = name;
    anchor.x = x;
    anchor.y = y;
    anchor.z = z;
    anchor.uncertainty_m = uncertainty_m;
    anchor.active = true;
    anchor.last_checked_timestamp = 0;
    
    anchors_[anchor_id] = anchor;
}

void TruthAnchorManager::removeAnchor(uint64_t anchor_id) {
    anchors_.erase(anchor_id);
}

void TruthAnchorManager::updateAnchor(uint64_t anchor_id, float x, float y, float z) {
    auto it = anchors_.find(anchor_id);
    if (it != anchors_.end()) {
        it->second.x = x;
        it->second.y = y;
        it->second.z = z;
    }
}

const TruthAnchor* TruthAnchorManager::getAnchor(uint64_t anchor_id) const {
    auto it = anchors_.find(anchor_id);
    return (it != anchors_.end()) ? &it->second : nullptr;
}

std::vector<TruthAnchor> TruthAnchorManager::getActiveAnchors() const {
    std::vector<TruthAnchor> result;
    for (const auto& pair : anchors_) {
        if (pair.second.active) {
            result.push_back(pair.second);
        }
    }
    return result;
}

size_t TruthAnchorManager::getAnchorCount() const {
    return anchors_.size();
}

void TruthAnchorManager::loadAnchors(const std::vector<TruthAnchor>& anchors) {
    for (const auto& anchor : anchors) {
        anchors_[anchor.anchor_id] = anchor;
    }
}

std::vector<TruthAnchor> TruthAnchorManager::exportAnchors() const {
    std::vector<TruthAnchor> result;
    for (const auto& pair : anchors_) {
        result.push_back(pair.second);
    }
    return result;
}

bool TruthAnchorManager::isValidAnchor(uint64_t anchor_id) const {
    auto it = anchors_.find(anchor_id);
    return (it != anchors_.end()) && it->second.active;
}

} // namespace ground_truth