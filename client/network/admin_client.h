#pragma once
#include <memory>
#include <grpcpp/grpcpp.h>
#include "admin.grpc.pb.h"

class AdminClient {
public:
    AdminClient(std::shared_ptr<grpc::Channel> channel);

    admin::GetUsersResponse getUsers(int page, int pageSize, const std::string& search = "");
    admin::User getUser(int id);
    admin::User createUser(const std::string& username, const std::string& password,
                           const std::string& email, const std::string& name, int roleId);
    admin::User updateUser(int id, const std::string& email, const std::string& name,
                           int roleId, bool active);
    void deleteUser(int id);

    admin::GetRolesResponse getRoles();
    admin::Role getRole(int id);
    admin::Role createRole(const std::string& name, const std::string& description);
    admin::Role updateRole(int id, const std::string& name, const std::string& description, bool active);
    void deleteRole(int id);

    admin::GetPermissionsResponse getPermissions();

    admin::SystemSettings getSystemSettings();
    admin::SystemSettings updateSystemSettings(const std::string& systemName,
                                                int sessionTimeout,
                                                bool enableAuditLog,
                                                bool enableNotifications);

private:
    std::unique_ptr<admin::AdminService::Stub> stub_;
};
