#pragma once
#include <string>
#include <vector>
#include <unordered_set>

struct User {
    int id;
    std::string username;
    std::string password_hash;
    std::string real_name;
    std::string email;
    std::string status;
};

struct Role {
    int id;
    std::string name;
    std::string description;
};

class AuthService {
public:
    static AuthService& instance();

    int createUser(const std::string& username, const std::string& password, const std::string& realName, const std::string& email);
    bool deleteUser(int userId);
    bool updateUserStatus(int userId, const std::string& status);
    
    User getUser(int userId);
    User getUserByUsername(const std::string& username);
    std::vector<User> listUsers();

    bool checkPermission(int userId, const std::string& permission);
    std::unordered_set<std::string> getUserPermissions(int userId);
    
    bool assignRole(int userId, int roleId);
    bool removeRole(int userId, int roleId);
    std::vector<Role> getUserRoles(int userId);

    bool authenticate(const std::string& username, const std::string& password);

private:
    AuthService() = default;
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;
};

class PermissionChecker {
public:
    static bool canCreateAsset(int userId);
    static bool canUpdateAsset(int userId);
    static bool canDeleteAsset(int userId);
    static bool canViewAsset(int userId);
    static bool canCreateInventoryTask(int userId);
    static bool canStartInventory(int userId);
    static bool canViewInventory(int userId);
    static bool canManageUsers(int userId);
};