#include "auth_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <openssl/sha.h.h>
#include <sstream>

namespace {

std::string sha256(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

}

AuthService& AuthService::instance() {
    static AuthService instance;
    return instance;
}

int AuthService::createUser(const std::string& username, const std::string& password,
                           const std::string& realName, const std::string& email) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::string passwordHash = sha256(password);

    pqxx::result R = W.exec(
        "INSERT INTO users(username, password_hash, real_name, email) VALUES(" +
        W.quote(username) + ", " +
        W.quote(passwordHash) + ", " +
        W.quote(realName) + ", " +
        W.quote(email) + ") RETURNING id"
    );

    int userId = R[0][0].as<int>();
    W.commit();

    spdlog::info("User created: username={}, id={}", username, userId);
    DBPool::instance().release(conn);
    return userId;
}

bool AuthService::deleteUser(int userId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    W.exec("DELETE FROM users WHERE id = " + W.to_string(userId));
    W.commit();

    spdlog::info("User deleted: id={}", userId);
    DBPool::instance().release(conn);
    return true;
}

bool AuthService::updateUserStatus(int userId, const std::string& status) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    W.exec("UPDATE users SET status = " + W.quote(status) + " WHERE id = " + W.to_string(userId));
    W.commit();

    spdlog::info("User status updated: id={}, status={}", userId, status);
    DBPool::instance().release(conn);
    return true;
}

User AuthService::getUser(int userId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT id, username, password_hash, real_name, email, status FROM users WHERE id = " + W.to_string(userId)
    );

    User user{};
    if (!R.empty()) {
        user.id = R[0][0].as<int>();
        user.username = R[0][1].as<std::string>();
        user.password_hash = R[0][2].as<std::string>();
        user.real_name = R[0][3].as<std::string>();
        user.email = R[0][4].as<std::string>();
        user.status = R[0][5].as<std::string>();
    }

    DBPool::instance().release(conn);
    return user;
}

User AuthService::getUserByUsername(const std::string& username) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT id, username, password_hash, real_name, email, status FROM users WHERE username = " + W.quote(username)
    );

    User user{};
    if (!R.empty()) {
        user.id = R[0][0].as<int>();
        user.username = R[0][1].as<std::string>();
        user.password_hash = R[0][2].as<std::string>();
        user.real_name = R[0][3].as<std::string>();
        user.email = R[0][4].as<std::string>();
        user.status = R[0][5].as<std::string>();
    }

    DBPool::instance().release(conn);
    return user;
}

std::vector<User> AuthService::listUsers() {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT id, username, password_hash, real_name, email, status FROM users"
    );

    std::vector<User> users;
    for (const auto& row : R) {
        User user;
        user.id = row[0].as<int>();
        user.username = row[1].as<std::string>();
        user.password_hash = row[2].as<std::string>();
        user.real_name = row[3].as<std::string>();
        user.email = row[4].as<std::string>();
        user.status = row[5].as<std::string>();
        users.push_back(user);
    }

    DBPool::instance().release(conn);
    return users;
}

bool AuthService::checkPermission(int userId, const std::string& permission) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT COUNT(*) FROM user_roles ur "
        "JOIN role_permissions rp ON ur.role_id = rp.role_id "
        "JOIN permissions p ON rp.permission_id = p.id "
        "WHERE ur.user_id = " + W.to_string(userId) + " AND p.name = " + W.quote(permission)
    );

    bool hasPermission = R[0][0].as<int>() > 0;

    DBPool::instance().release(conn);
    return hasPermission;
}

std::unordered_set<std::string> AuthService::getUserPermissions(int userId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT DISTINCT p.name FROM user_roles ur "
        "JOIN role_permissions rp ON ur.role_id = rp.role_id "
        "JOIN permissions p ON rp.permission_id = p.id "
        "WHERE ur.user_id = " + W.to_string(userId)
    );

    std::unordered_set<std::string> permissions;
    for (const auto& row : R) {
        permissions.insert(row[0].as<std::string>());
    }

    DBPool::instance().release(conn);
    return permissions;
}

bool AuthService::assignRole(int userId, int roleId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    try {
        W.exec(
            "INSERT INTO user_roles(user_id, role_id) VALUES(" +
            W.to_string(userId) + ", " + W.to_string(roleId) + ")"
        );
        W.commit();
        spdlog::info("Role assigned: userId={}, roleId={}", userId, roleId);
        DBPool::instance().release(conn);
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to assign role: {}", e.what());
        DBPool::instance().release(conn);
        return false;
    }
}

bool AuthService::removeRole(int userId, int roleId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    W.exec(
        "DELETE FROM user_roles WHERE user_id = " + W.to_string(userId) +
        " AND role_id = " + W.to_string(roleId)
    );
    W.commit();

    spdlog::info("Role removed: userId={}, roleId={}", userId, roleId);
    DBPool::instance().release(conn);
    return true;
}

std::vector<Role> AuthService::getUserRoles(int userId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT r.id, r.name, r.description FROM roles r "
        "JOIN user_roles ur ON r.id = ur.role_id "
        "WHERE ur.user_id = " + W.to_string(userId)
    );

    std::vector<Role> roles;
    for (const auto& row : R) {
        Role role;
        role.id = row[0].as<int>();
        role.name = row[1].as<std::string>();
        role.description = row[2].as<std::string>();
        roles.push_back(role);
    }

    DBPool::instance().release(conn);
    return roles;
}

bool AuthService::authenticate(const std::string& username, const std::string& password) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::string passwordHash = sha256(password);

    pqxx::result R = W.exec(
        "SELECT COUNT(*) FROM users WHERE username = " + W.quote(username) +
        " AND password_hash = " + W.quote(passwordHash) + " AND status = 'ACTIVE'"
    );

    bool authenticated = R[0][0].as<int>() > 0;

    DBPool::instance().release(conn);
    return authenticated;
}

bool PermissionChecker::canCreateAsset(int userId) {
    return AuthService::instance().checkPermission(userId, "asset:create");
}

bool PermissionChecker::canUpdateAsset(int userId) {
    return AuthService::instance().checkPermission(userId, "asset:update");
}

bool PermissionChecker::canDeleteAsset(int userId) {
    return AuthService::instance().checkPermission(userId, "asset:delete");
}

bool PermissionChecker::canViewAsset(int userId) {
    return AuthService::instance().checkPermission(userId, "asset:read");
}

bool PermissionChecker::canCreateInventoryTask(int userId) {
    return AuthService::instance().checkPermission(userId, "inventory:create");
}

bool PermissionChecker::canStartInventory(int userId) {
    return AuthService::instance().checkPermission(userId, "inventory:start");
}

bool PermissionChecker::canViewInventory(int userId) {
    return AuthService::instance().checkPermission(userId, "inventory:view");
}

bool PermissionChecker::canManageUsers(int userId) {
    return AuthService::instance().checkPermission(userId, "user:manage");
}