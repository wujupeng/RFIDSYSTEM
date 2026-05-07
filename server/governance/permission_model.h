#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum class RuntimePermission {
    READ_ONLY,
    MODIFY_POLICY,
    MODIFY_TOPOLOGY,
    CONTROL_GPU,
    FACTORY_ISOLATION,
    EMERGENCY_STOP
};

enum class AIRole {
    OBSERVER,
    OPTIMIZER,
    HEALER,
    ADMINISTRATOR
};

class PermissionModel {
public:
    PermissionModel();
    
    bool hasPermission(AIRole role, RuntimePermission permission);
    
    void grantPermission(AIRole role, RuntimePermission permission);
    
    void revokePermission(AIRole role, RuntimePermission permission);
    
    void setRole(AIRole role);
    
    AIRole getCurrentRole() const;

private:
    std::vector<RuntimePermission> getPermissionsForRole(AIRole role);
    
    AIRole current_role_;
};