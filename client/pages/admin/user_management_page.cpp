#include "user_management_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>

UserManagementPage::UserManagementPage(AdminClient* client, QWidget* parent)
    : QWidget(parent), client_(client), currentPage_(1)
{
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索用户名、邮箱或姓名...");
    connect(searchEdit_, &QLineEdit::returnPressed, this, &UserManagementPage::searchUsers);
    
    auto* searchBtn = new QPushButton("搜索");
    connect(searchBtn, &QPushButton::clicked, this, &UserManagementPage::searchUsers);
    
    addBtn_ = new QPushButton("添加用户");
    connect(addBtn_, &QPushButton::clicked, this, &UserManagementPage::addUser);
    
    topLayout->addWidget(searchEdit_);
    topLayout->addWidget(searchBtn);
    topLayout->addStretch();
    topLayout->addWidget(addBtn_);
    mainLayout->addLayout(topLayout);

    table_ = new QTableWidget();
    table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels({"ID", "用户名", "邮箱", "姓名", "角色", "状态", "创建时间"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(table_);

    auto* bottomLayout = new QHBoxLayout();
    editBtn_ = new QPushButton("编辑");
    deleteBtn_ = new QPushButton("删除");
    
    connect(editBtn_, &QPushButton::clicked, this, &UserManagementPage::editUser);
    connect(deleteBtn_, &QPushButton::clicked, this, &UserManagementPage::deleteUser);
    
    bottomLayout->addWidget(editBtn_);
    bottomLayout->addWidget(deleteBtn_);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    loadUsers();
}

void UserManagementPage::loadUsers() {
    try {
        auto resp = client_->getUsers(currentPage_, 20, searchEdit_->text().toStdString());
        
        table_->setRowCount(0);
        for (const auto& user : resp.users()) {
            int row = table_->rowCount();
            table_->insertRow(row);
            
            table_->setItem(row, 0, new QTableWidgetItem(QString::number(user.id())));
            table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.username())));
            table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(user.email())));
            table_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(user.name())));
            table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(user.role_name())));
            table_->setItem(row, 5, new QTableWidgetItem(user.active() ? "启用" : "禁用"));
            table_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(user.created_at())));
            
            if (!user.active()) {
                for (int col = 0; col < 7; ++col) {
                    table_->item(row, col)->setForeground(QColor(Qt::gray));
                }
            }
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载用户失败: %1").arg(e.what()));
    }
}

void UserManagementPage::searchUsers() {
    currentPage_ = 1;
    loadUsers();
}

void UserManagementPage::addUser() {
    showUserDialog();
}

void UserManagementPage::editUser() {
    auto selected = table_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要编辑的用户");
        return;
    }
    
    int userId = selected[0]->text().toInt();
    try {
        auto user = client_->getUser(userId);
        showUserDialog(&user);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("获取用户信息失败: %1").arg(e.what()));
    }
}

void UserManagementPage::deleteUser() {
    auto selected = table_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要删除的用户");
        return;
    }
    
    int userId = selected[0]->text().toInt();
    QString username = selected[1]->text();
    
    if (QMessageBox::question(this, "确认删除", 
        QString("确定要删除用户 \"%1\" 吗？").arg(username)) == QMessageBox::Yes) {
        try {
            client_->deleteUser(userId);
            QMessageBox::information(this, "成功", "用户删除成功");
            loadUsers();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "错误", QString("删除用户失败: %1").arg(e.what()));
        }
    }
}

void UserManagementPage::showUserDialog(const admin::User* user) {
    QDialog dialog(this);
    dialog.setWindowTitle(user ? "编辑用户" : "添加用户");
    dialog.resize(400, 350);

    auto* layout = new QFormLayout(&dialog);

    QLineEdit* usernameEdit = new QLineEdit();
    QLineEdit* passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    QLineEdit* emailEdit = new QLineEdit();
    QLineEdit* nameEdit = new QLineEdit();
    QComboBox* roleCombo = new QComboBox();
    QCheckBox* activeCheck = new QCheckBox("启用");
    activeCheck->setChecked(true);

    try {
        auto roles = client_->getRoles();
        for (const auto& role : roles.roles()) {
            roleCombo->addItem(QString::fromStdString(role.name()), role.id());
        }
    } catch (const std::exception&) {
    }

    if (user) {
        usernameEdit->setText(QString::fromStdString(user->username()));
        usernameEdit->setReadOnly(true);
        emailEdit->setText(QString::fromStdString(user->email()));
        nameEdit->setText(QString::fromStdString(user->name()));
        roleCombo->setCurrentIndex(roleCombo->findData(user->role_id()));
        activeCheck->setChecked(user->active());
    } else {
        passwordEdit->setPlaceholderText("请输入密码");
    }

    layout->addRow("用户名:", usernameEdit);
    if (!user) layout->addRow("密码:", passwordEdit);
    layout->addRow("邮箱:", emailEdit);
    layout->addRow("姓名:", nameEdit);
    layout->addRow("角色:", roleCombo);
    layout->addRow(activeCheck);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            if (user) {
                client_->updateUser(
                    user->id(),
                    emailEdit->text().toStdString(),
                    nameEdit->text().toStdString(),
                    roleCombo->currentData().toInt(),
                    activeCheck->isChecked()
                );
                QMessageBox::information(this, "成功", "用户信息更新成功");
            } else {
                client_->createUser(
                    usernameEdit->text().toStdString(),
                    passwordEdit->text().toStdString(),
                    emailEdit->text().toStdString(),
                    nameEdit->text().toStdString(),
                    roleCombo->currentData().toInt()
                );
                QMessageBox::information(this, "成功", "用户创建成功");
            }
            loadUsers();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "错误", QString("操作失败: %1").arg(e.what()));
        }
    }
}
