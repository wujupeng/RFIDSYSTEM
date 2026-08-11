#include "coordinate_system_manager.h"
#include <algorithm>

namespace pa::spatial_truth {

CoordinateSystemManager& CoordinateSystemManager::instance() {
    static CoordinateSystemManager inst;
    return inst;
}

CoordinateSystemManager::CoordinateSystemManager() {
    initDefault();
}

void CoordinateSystemManager::initDefault() {
    SpatialCoordinateSystem global;
    global.id = "global";
    global.parent_id = "";
    global.origin_x = 0.0;
    global.origin_y = 0.0;
    global.origin_z = 0.0;
    global.rotation = 0.0;
    global.unit = CoordinateUnit::METER;
    global.version = 1;
    systems_by_id_["global"] = {global};
}

bool CoordinateSystemManager::wouldCreateCycle(const std::string& id, const std::string& parent_id) const {
    if (id == parent_id) return true;
    if (parent_id.empty()) return false;

    std::string current = parent_id;
    int max_depth = 10;
    while (!current.empty() && max_depth-- > 0) {
        if (current == id) return true;
        auto it = systems_by_id_.find(current);
        if (it == systems_by_id_.end() || it->second.empty()) break;
        current = it->second.back().parent_id;
    }
    return false;
}

bool CoordinateSystemManager::isValidUnit(CoordinateUnit unit) const {
    return unit == CoordinateUnit::METER ||
           unit == CoordinateUnit::CENTIMETER ||
           unit == CoordinateUnit::MILLIMETER;
}

CreateCSResult CoordinateSystemManager::create(const SpatialCoordinateSystem& cs) {
    std::lock_guard<std::mutex> lock(mutex_);

    CreateCSResult result;

    if (systems_by_id_.count(cs.id) && !cs.id.empty()) {
        result.error = CoordinateSystemError::ALREADY_EXISTS;
        result.error_message = "Coordinate system already exists: " + cs.id;
        return result;
    }

    if (!cs.parent_id.empty()) {
        auto it = systems_by_id_.find(cs.parent_id);
        if (it == systems_by_id_.end() || it->second.empty()) {
            result.error = CoordinateSystemError::PARENT_NOT_FOUND;
            result.error_message = "Parent coordinate system not found: " + cs.parent_id;
            return result;
        }
    }

    if (wouldCreateCycle(cs.id, cs.parent_id)) {
        result.error = CoordinateSystemError::CYCLIC_PARENT_REFERENCE;
        result.error_message = "Cyclic parent reference detected";
        return result;
    }

    if (!isValidUnit(cs.unit)) {
        result.error = CoordinateSystemError::INVALID_UNIT;
        result.error_message = "Invalid unit";
        return result;
    }

    SpatialCoordinateSystem new_cs = cs;
    new_cs.version = next_version_.fetch_add(1);
    systems_by_id_[cs.id] = {new_cs};

    result.id = cs.id;
    result.version = new_cs.version;
    return result;
}

CreateCSResult CoordinateSystemManager::update(const std::string& id, const SpatialCoordinateSystem& cs) {
    std::lock_guard<std::mutex> lock(mutex_);

    CreateCSResult result;

    auto it = systems_by_id_.find(id);
    if (it == systems_by_id_.end() || it->second.empty()) {
        result.error = CoordinateSystemError::COORDINATE_SYSTEM_NOT_FOUND;
        result.error_message = "Coordinate system not found: " + id;
        return result;
    }

    if (!cs.parent_id.empty() && cs.parent_id != id) {
        if (wouldCreateCycle(id, cs.parent_id)) {
            result.error = CoordinateSystemError::CYCLIC_PARENT_REFERENCE;
            result.error_message = "Cyclic parent reference detected";
            return result;
        }
        auto pit = systems_by_id_.find(cs.parent_id);
        if (pit == systems_by_id_.end() || pit->second.empty()) {
            result.error = CoordinateSystemError::PARENT_NOT_FOUND;
            result.error_message = "Parent coordinate system not found: " + cs.parent_id;
            return result;
        }
    }

    if (!isValidUnit(cs.unit)) {
        result.error = CoordinateSystemError::INVALID_UNIT;
        result.error_message = "Invalid unit";
        return result;
    }

    SpatialCoordinateSystem new_cs = cs;
    new_cs.id = id;
    new_cs.version = next_version_.fetch_add(1);
    it->second.push_back(new_cs);

    result.id = id;
    result.version = new_cs.version;
    return result;
}

std::optional<SpatialCoordinateSystem> CoordinateSystemManager::get(const std::string& id, uint64_t version) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = systems_by_id_.find(id);
    if (it == systems_by_id_.end() || it->second.empty()) return std::nullopt;

    if (version == 0) return it->second.back();

    for (const auto& cs : it->second) {
        if (cs.version == version) return cs;
    }
    return std::nullopt;
}

bool CoordinateSystemManager::isRegistered(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return systems_by_id_.count(id) > 0;
}

std::vector<SpatialCoordinateSystem> CoordinateSystemManager::listByParent(const std::string& parent_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SpatialCoordinateSystem> result;
    for (const auto& [id, versions] : systems_by_id_) {
        if (!versions.empty() && versions.back().parent_id == parent_id) {
            result.push_back(versions.back());
        }
    }
    return result;
}

HierarchyNode CoordinateSystemManager::getHierarchyTree(const std::string& root_id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    HierarchyNode root;
    auto it = systems_by_id_.find(root_id);
    if (it == systems_by_id_.end() || it->second.empty()) return root;

    root.cs = it->second.back();

    auto children = listByParent(root_id);
    for (const auto& child : children) {
        root.children.push_back(getHierarchyTree(child.id));
    }
    return root;
}

} // namespace pa::spatial_truth