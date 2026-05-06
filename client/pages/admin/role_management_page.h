#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include "../../network/admin_client.h"

class RoleManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit RoleManagementPage(AdminClient* client, QWidget* parent = nullptr);

private slots:
    void loadRoles();
    void addRole();
    void editRole();
    void deleteRole();
    void showRoleDialog(const admin::Role* role = nullptr);

private:
    AdminClient* client_;
    QTableWidget* table_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};
