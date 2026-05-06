#include "admin_client.h"
#include <chrono>

AdminClient::AdminClient(std::shared_ptr<grpc::Channel> channel) {
    stub_ = admin::AdminService::NewStub(channel);
}

admin::GetUsersResponse AdminClient::getUsers(int page, int pageSize, const std::string& search) {
    admin::GetUsersRequest req;
    req.set_page(page);
    req.set_page_size(pageSize);
    req.set_search(search);
    
    admin::GetUsersResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->GetUsers(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("GetUsers failed: " + status.error_message());
    }
    return resp;
}

admin::User AdminClient::getUser(int id) {
    admin::GetUserRequest req;
    req.set_id(id);
    
    admin::User resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->GetUser(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("GetUser failed: " + status.error_message());
    }
    return resp;
}

admin::User AdminClient::createUser(const std::string& username, const std::string& password,
                                   const std::string& email, const std::string& name, int roleId) {
    admin::CreateUserRequest req;
    req.set_username(username);
    req.set_password(password);
    req.set_email(email);
    req.set_name(name);
    req.set_role_id(roleId);
    
    admin::User resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->CreateUser(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("CreateUser failed: " + status.error_message());
    }
    return resp;
}

admin::User AdminClient::updateUser(int id, const std::string& email, const std::string& name,
                                   int roleId, bool active) {
    admin::UpdateUserRequest req;
    req.set_id(id);
    req.set_email(email);
    req.set_name(name);
    req.set_role_id(roleId);
    req.set_active(active);
    
    admin::User resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->UpdateUser(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("UpdateUser failed: " + status.error_message());
    }
    return resp;
}

void AdminClient::deleteUser(int id) {
    admin::DeleteUserRequest req;
    req.set_id(id);
    
    admin::EmptyResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->DeleteUser(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("DeleteUser failed: " + status.error_message());
    }
}

admin::GetRolesResponse AdminClient::getRoles() {
    admin::GetRolesRequest req;
    
    admin::GetRolesResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->GetRoles(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("GetRoles failed: " + status.error_message());
    }
    return resp;
}

admin::Role AdminClient::getRole(int id) {
    admin::GetRoleRequest req;
    req.set_id(id);
    
    admin::Role resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->GetRole(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("GetRole failed: " + status.error_message());
    }
    return resp;
}

admin::Role AdminClient::createRole(const std::string& name, const std::string& description) {
    admin::CreateRoleRequest req;
    req.set_name(name);
    req.set_description(description);
    
    admin::Role resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->CreateRole(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("CreateRole failed: " + status.error_message());
    }
    return resp;
}

admin::Role AdminClient::updateRole(int id, const std::string& name, const std::string& description, bool active) {
    admin::UpdateRoleRequest req;
    req.set_id(id);
    req.set_name(name);
    req.set_description(description);
    req.set_active(active);
    
    admin::Role resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->UpdateRole(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("UpdateRole failed: " + status.error_message());
    }
    return resp;
}

void AdminClient::deleteRole(int id) {
    admin::DeleteRoleRequest req;
    req.set_id(id);
    
    admin::EmptyResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    grpc::Status status = stub_->DeleteRole(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("DeleteRole failed: " + status.error_message());
    }
}

admin::GetPermissionsResponse AdminClient::getPermissions() {
    admin::GetPermissionsRequest req;
    
    admin::GetPermissionsResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    stub_->GetPermissions(&ctx, req, &resp);
    return resp;
}

admin::SystemSettings AdminClient::getSystemSettings() {
    admin::EmptyRequest req;
    
    admin::SystemSettings resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    stub_->GetSystemSettings(&ctx, req, &resp);
    return resp;
}

admin::SystemSettings AdminClient::updateSystemSettings(const std::string& systemName,
                                                        int sessionTimeout,
                                                        bool enableAuditLog,
                                                        bool enableNotifications) {
    admin::UpdateSystemSettingsRequest req;
    req.set_system_name(systemName);
    req.set_session_timeout_minutes(sessionTimeout);
    req.set_enable_audit_log(enableAuditLog);
    req.set_enable_notifications(enableNotifications);
    
    admin::SystemSettings resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    ctx.set_deadline(deadline);
    
    stub_->UpdateSystemSettings(&ctx, req, &resp);
    return resp;
}
