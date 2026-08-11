#pragma once

#include "spatial_coordinate_system.h"
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <optional>
#include <atomic>

namespace pa::spatial_truth {

enum class CoordinateSystemError {
    NONE,
    CYCLIC_PARENT_REFERENCE,
    PARENT_NOT_FOUND,
    INVALID_UNIT,
    COORDINATE_SYSTEM_NOT_FOUND,
    ALREADY_EXISTS,
};

struct CreateCSResult {
    CoordinateSystemError error = CoordinateSystemError::NONE;
    std::string error_message;
    std::string id;
    uint64_t version = 0;
};

struct HierarchyNode {
    SpatialCoordinateSystem cs;
    std::vector<HierarchyNode> children;
};

class CoordinateSystemManager {
public:
    static CoordinateSystemManager& instance();

    CreateCSResult create(const SpatialCoordinateSystem& cs);
    CreateCSResult update(const std::string& id, const SpatialCoordinateSystem& cs);
    std::optional<SpatialCoordinateSystem> get(const std::string& id, uint64_t version = 0) const;
    bool isRegistered(const std::string& id) const;
    std::vector<SpatialCoordinateSystem> listByParent(const std::string& parent_id) const;
    HierarchyNode getHierarchyTree(const std::string& root_id = "global") const;

private:
    CoordinateSystemManager();

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<SpatialCoordinateSystem>> systems_by_id_;
    std::atomic<uint64_t> next_version_{1};

    bool wouldCreateCycle(const std::string& id, const std::string& parent_id) const;
    bool isValidUnit(CoordinateUnit unit) const;
    void initDefault();
};

} // namespace pa::spatial_truth