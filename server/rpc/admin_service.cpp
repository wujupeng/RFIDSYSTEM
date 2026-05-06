#include "admin_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <sstream>

grpc::Status AdminServiceImpl::GetUsers(
    grpc::ServerContext* context,
    const admin::GetUsersRequest* request,
    admin::GetUsersResponse* response)
{
    spdlog::info("RPC: GetUsers called");

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        int offset = (request->page() - 1) * request->page_size();
        std::string search = request->search();

        std::stringstream query;
        query << "SELECT u.*, r.name as role_name FROM users u "
              << "LEFT JOIN roles r ON u.role_id = r.id ";

        if (!search.empty()) {
            query << "WHERE u.username LIKE '%" << W.esc(search) << "%' "
                  << "OR u.email LIKE '%" << W.esc(search) << "%' "
                  << "OR u.name LIKE '%" << W.esc(search) << "%' ";
        }

        query << "ORDER BY u.created_at DESC "
              << "LIMIT " << request->page_size() << " OFFSET " << offset;

        pqxx::result R = W.exec(query.str());

        for (const auto& row : R) {
            admin::User* user = response->add_users();
            user->set_id(row["id"].as<int>());
            user->set_username(row["username"].as<std::string>());
            user->set_email(row["email"].as<std::string>());
            user->set_name(row["name"].as<std::string>());
            user->set_role_id(row["role_id"].as<int>());
            user->set_role_name(row["role_name"].as<std::string>());
            user->set_active(row["active"].as<bool>());
            user->set_created_at(row["created_at"].as<std::string>());
            user->set_updated_at(row["updated_at"].as<std::string>());
        }

        std::string countQuery = "SELECT COUNT(*) FROM users";
        if (!search.empty()) {
            countQuery += " WHERE username LIKE '%" + W.esc(search) + "%' OR email LIKE '%" + W.esc(search) + "%'";
        }
        pqxx::result countResult = W.exec(countQuery);
        response->set_total(countResult[0][0].as<int>());

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetUsers failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::GetUser(
    grpc::ServerContext* context,
    const admin::GetUserRequest* request,
    admin::User* response)
{
    spdlog::info("RPC: GetUser called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT u.*, r.name as role_name FROM users u "
            "LEFT JOIN roles r ON u.role_id = r.id "
            "WHERE u.id = " + std::to_string(request->id())
        );

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_username(row["username"].as<std::string>());
            response->set_email(row["email"].as<std::string>());
            response->set_name(row["name"].as<std::string>());
            response->set_role_id(row["role_id"].as<int>());
            response->set_role_name(row["role_name"].as<std::string>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
            response->set_updated_at(row["updated_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetUser failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::CreateUser(
    grpc::ServerContext* context,
    const admin::CreateUserRequest* request,
    admin::User* response)
{
    spdlog::info("RPC: CreateUser called - username={}", request->username());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "INSERT INTO users (username, password, email, name, role_id, active) "
            "VALUES ('" + W.esc(request->username()) + "', "
            "'" + W.esc(request->password()) + "', "
            "'" + W.esc(request->email()) + "', "
            "'" + W.esc(request->name()) + "', "
            "" + std::to_string(request->role_id()) + ", "
            "true) RETURNING *"
        );

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_username(row["username"].as<std::string>());
            response->set_email(row["email"].as<std::string>());
            response->set_name(row["name"].as<std::string>());
            response->set_role_id(row["role_id"].as<int>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
            response->set_updated_at(row["updated_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: CreateUser failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::UpdateUser(
    grpc::ServerContext* context,
    const admin::UpdateUserRequest* request,
    admin::User* response)
{
    spdlog::info("RPC: UpdateUser called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "UPDATE users SET "
            "email = '" + W.esc(request->email()) + "', "
            "name = '" + W.esc(request->name()) + "', "
            "role_id = " + std::to_string(request->role_id()) + ", "
            "active = " + (request->active() ? "true" : "false") + ", "
            "updated_at = NOW() "
            "WHERE id = " + std::to_string(request->id()) + " RETURNING *"
        );

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_username(row["username"].as<std::string>());
            response->set_email(row["email"].as<std::string>());
            response->set_name(row["name"].as<std::string>());
            response->set_role_id(row["role_id"].as<int>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
            response->set_updated_at(row["updated_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: UpdateUser failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::DeleteUser(
    grpc::ServerContext* context,
    const admin::DeleteUserRequest* request,
    admin::EmptyResponse* response)
{
    spdlog::info("RPC: DeleteUser called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        W.exec("DELETE FROM users WHERE id = " + std::to_string(request->id()));
        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: DeleteUser failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::GetRoles(
    grpc::ServerContext* context,
    const admin::GetRolesRequest* request,
    admin::GetRolesResponse* response)
{
    spdlog::info("RPC: GetRoles called");

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec("SELECT * FROM roles ORDER BY name");

        for (const auto& row : R) {
            admin::Role* role = response->add_roles();
            role->set_id(row["id"].as<int>());
            role->set_name(row["name"].as<std::string>());
            role->set_description(row["description"].as<std::string>());
            role->set_active(row["active"].as<bool>());
            role->set_created_at(row["created_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetRoles failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::GetRole(
    grpc::ServerContext* context,
    const admin::GetRoleRequest* request,
    admin::Role* response)
{
    spdlog::info("RPC: GetRole called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec("SELECT * FROM roles WHERE id = " + std::to_string(request->id()));

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_name(row["name"].as<std::string>());
            response->set_description(row["description"].as<std::string>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetRole failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::CreateRole(
    grpc::ServerContext* context,
    const admin::CreateRoleRequest* request,
    admin::Role* response)
{
    spdlog::info("RPC: CreateRole called - name={}", request->name());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "INSERT INTO roles (name, description, active) "
            "VALUES ('" + W.esc(request->name()) + "', "
            "'" + W.esc(request->description()) + "', "
            "true) RETURNING *"
        );

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_name(row["name"].as<std::string>());
            response->set_description(row["description"].as<std::string>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: CreateRole failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::UpdateRole(
    grpc::ServerContext* context,
    const admin::UpdateRoleRequest* request,
    admin::Role* response)
{
    spdlog::info("RPC: UpdateRole called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "UPDATE roles SET "
            "name = '" + W.esc(request->name()) + "', "
            "description = '" + W.esc(request->description()) + "', "
            "active = " + (request->active() ? "true" : "false") + " "
            "WHERE id = " + std::to_string(request->id()) + " RETURNING *"
        );

        if (!R.empty()) {
            const auto& row = R[0];
            response->set_id(row["id"].as<int>());
            response->set_name(row["name"].as<std::string>());
            response->set_description(row["description"].as<std::string>());
            response->set_active(row["active"].as<bool>());
            response->set_created_at(row["created_at"].as<std::string>());
        }

        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: UpdateRole failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::DeleteRole(
    grpc::ServerContext* context,
    const admin::DeleteRoleRequest* request,
    admin::EmptyResponse* response)
{
    spdlog::info("RPC: DeleteRole called - id={}", request->id());

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        W.exec("DELETE FROM roles WHERE id = " + std::to_string(request->id()));
        W.commit();
        DBPool::instance().release(conn);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: DeleteRole failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AdminServiceImpl::GetPermissions(
    grpc::ServerContext* context,
    const admin::GetPermissionsRequest* request,
    admin::GetPermissionsResponse* response)
{
    spdlog::info("RPC: GetPermissions called");

    std::vector<std::tuple<std::string, std::string, std::string, std::string>> permissions = {
        {"asset.view", "查看资产", "资产模块", "asset"},
        {"asset.create", "创建资产", "资产模块", "asset"},
        {"asset.edit", "编辑资产", "资产模块", "asset"},
        {"asset.delete", "删除资产", "资产模块", "asset"},
        {"inventory.view", "查看盘点", "盘点模块", "inventory"},
        {"inventory.create", "创建盘点", "盘点模块", "inventory"},
        {"repair.view", "查看维修", "维修模块", "repair"},
        {"repair.create", "创建维修", "维修模块", "repair"},
        {"repair.approve", "审批维修", "维修模块", "repair"},
        {"admin.user.view", "查看用户", "管理模块", "admin"},
        {"admin.user.create", "创建用户", "管理模块", "admin"},
        {"admin.user.edit", "编辑用户", "管理模块", "admin"},
        {"admin.user.delete", "删除用户", "管理模块", "admin"},
        {"admin.role.view", "查看角色", "管理模块", "admin"},
        {"admin.role.create", "创建角色", "管理模块", "admin"},
        {"admin.role.edit", "编辑角色", "管理模块", "admin"},
        {"admin.role.delete", "删除角色", "管理模块", "admin"},
        {"admin.settings", "系统设置", "管理模块", "admin"},
        {"monitoring.view", "查看监控", "监控模块", "monitoring"},
        {"report.view", "查看报表", "报表模块", "report"}
    };

    for (const auto& [id, name, desc, category] : permissions) {
        admin::Permission* p = response->add_permissions();
        p->set_id(id);
        p->set_name(name);
        p->set_description(desc);
        p->set_category(category);
    }

    return grpc::Status::OK;
}

grpc::Status AdminServiceImpl::GetSystemSettings(
    grpc::ServerContext* context,
    const admin::EmptyRequest* request,
    admin::SystemSettings* response)
{
    spdlog::info("RPC: GetSystemSettings called");

    response->set_system_name("RFID资产管理系统");
    response->set_system_version("v3.3.1");
    response->set_session_timeout_minutes(30);
    response->set_enable_audit_log(true);
    response->set_enable_notifications(true);

    return grpc::Status::OK;
}

grpc::Status AdminServiceImpl::UpdateSystemSettings(
    grpc::ServerContext* context,
    const admin::UpdateSystemSettingsRequest* request,
    admin::SystemSettings* response)
{
    spdlog::info("RPC: UpdateSystemSettings called");

    response->set_system_name(request->system_name());
    response->set_system_version("v3.3.1");
    response->set_session_timeout_minutes(request->session_timeout_minutes());
    response->set_enable_audit_log(request->enable_audit_log());
    response->set_enable_notifications(request->enable_notifications());

    return grpc::Status::OK;
}
