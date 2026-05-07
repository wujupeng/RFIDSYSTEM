#include "permission_model.h"

PermissionModel::PermissionModel() : current_role_(AIRole::OBSERVER) {}

bool PermissionModel::hasPermission(AIRole role, RuntimePermission permission) {
    auto permissions = getPermissionsForRole(role);
    
    for (auto p : permissions) {
        if (p == permission) {
            return true;
        }
    }
    
    return false;
}

void PermissionModel::grantPermission(AIRole role, RuntimePermission permission) {
}

void PermissionModel::revokePermission(AIRole role, RuntimePermission permission) {
}

void PermissionModel::setRole(AIRole role) {
    current_role_ = role;
}

AIRole PermissionModel::getCurrentRole() const {
    return current_role_;
}

std::vector<RuntimePermission> PermissionModel::getPermissionsForRole(AIRole role) {
    std::vector<RuntimePermission> permissions;
    
    switch (role) {
        case AIRole::OBSERVER:
            permissions.push_back(RuntimePermission::READ_ONLY);
            break;
            
        case AIRole::OPTIMIZER:
            permissions.push_back(RuntimePermission::READ_ONLY);
            permissions.push_back(RuntimePermission::MODIFY_POLICY);
            permissions.push_back(RuntimePermission::MODIFY_TOPOLOGY);
            permissions.push_back(RuntimePermission::CONTROL_GPU);
            break;
            
        case AIRole::HEALER:
            permissions.push_back(RuntimePermission::READ_ONLY);
            permissions.push_back(RuntimePermission::FACTORY_ISOLATION);
            permissions.push_back(RuntimePermission::EMERGENCY_STOP);
            break;
            
        case AIRole::ADMINISTRATOR:
            permissions.push_back(RuntimePermission::READ_ONLY);
            permissions.push_back(RuntimePermission::MODIFY_POLICY);
            permissions.push_back(RuntimePermission::MODIFY_TOPOLOGY);
            permissions.push_back(RuntimePermission::CONTROL_GPU);
            permissions.push_back(RuntimePermission::FACTORY_ISOLATION);
            permissions.push_back(RuntimePermission::EMERGENCY_STOP);
            break;
    }
    
    return permissions;
}