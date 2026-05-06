#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QComboBox>
#include "../../network/admin_client.h"

class UserManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit UserManagementPage(AdminClient* client, QWidget* parent = nullptr);

private slots:
    void loadUsers();
    void searchUsers();
    void addUser();
    void editUser();
    void deleteUser();
    void showUserDialog(const admin::User* user = nullptr);

private:
    AdminClient* client_;
    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
    int currentPage_;
};
