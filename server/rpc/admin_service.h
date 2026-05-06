#pragma once
#include "admin.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class AdminServiceImpl final : public admin::AdminService::Service {
public:
    grpc::Status GetUsers(
        grpc::ServerContext* context,
        const admin::GetUsersRequest* request,
        admin::GetUsersResponse* response) override;

    grpc::Status GetUser(
        grpc::ServerContext* context,
        const admin::GetUserRequest* request,
        admin::User* response) override;

    grpc::Status CreateUser(
        grpc::ServerContext* context,
        const admin::CreateUserRequest* request,
        admin::User* response) override;

    grpc::Status UpdateUser(
        grpc::ServerContext* context,
        const admin::UpdateUserRequest* request,
        admin::User* response) override;

    grpc::Status DeleteUser(
        grpc::ServerContext* context,
        const admin::DeleteUserRequest* request,
        admin::EmptyResponse* response) override;

    grpc::Status GetRoles(
        grpc::ServerContext* context,
        const admin::GetRolesRequest* request,
        admin::GetRolesResponse* response) override;

    grpc::Status GetRole(
        grpc::ServerContext* context,
        const admin::GetRoleRequest* request,
        admin::Role* response) override;

    grpc::Status CreateRole(
        grpc::ServerContext* context,
        const admin::CreateRoleRequest* request,
        admin::Role* response) override;

    grpc::Status UpdateRole(
        grpc::ServerContext* context,
        const admin::UpdateRoleRequest* request,
        admin::Role* response) override;

    grpc::Status DeleteRole(
        grpc::ServerContext* context,
        const admin::DeleteRoleRequest* request,
        admin::EmptyResponse* response) override;

    grpc::Status GetPermissions(
        grpc::ServerContext* context,
        const admin::GetPermissionsRequest* request,
        admin::GetPermissionsResponse* response) override;

    grpc::Status GetSystemSettings(
        grpc::ServerContext* context,
        const admin::EmptyRequest* request,
        admin::SystemSettings* response) override;

    grpc::Status UpdateSystemSettings(
        grpc::ServerContext* context,
        const admin::UpdateSystemSettingsRequest* request,
        admin::SystemSettings* response) override;
};
