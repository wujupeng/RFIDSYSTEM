#include "role_management_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>

RoleManagementPage::RoleManagementPage(AdminClient* client, QWidget* parent)
    : QWidget(parent), client_(client)
{
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    addBtn_ = new QPushButton("添加角色");
    connect(addBtn_, &QPushButton::clicked, this, &RoleManagementPage::addRole);
    topLayout->addStretch();
    topLayout->addWidget(addBtn_);
    mainLayout->addLayout(topLayout);

    table_ = new QTableWidget();
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({"ID", "角色名称", "描述", "状态"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(table_);

    auto* bottomLayout = new QHBoxLayout();
    editBtn_ = new QPushButton("编辑");
    deleteBtn_ = new QPushButton("删除");
    
    connect(editBtn_, &QPushButton::clicked, this, &RoleManagementPage::editRole);
    connect(deleteBtn_, &QPushButton::clicked, this, &RoleManagementPage::deleteRole);
    
    bottomLayout->addWidget(editBtn_);
    bottomLayout->addWidget(deleteBtn_);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    loadRoles();
}

void RoleManagementPage::loadRoles() {
    try {
        auto resp = client_->getRoles();
        
        table_->setRowCount(0);
        for (const auto& role : resp.roles()) {
            int row = table_->rowCount();
            table_->insertRow(row);
            
            table_->setItem(row, 0, new QTableWidgetItem(QString::number(role.id())));
            table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(role.name())));
            table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(role.description())));
            table_->setItem(row, 3, new QTableWidgetItem(role.active() ? "启用" : "禁用"));
            
            if (!role.active()) {
                for (int col = 0; col < 4; ++col) {
                    table_->item(row, col)->setForeground(QColor(Qt::gray));
                }
            }
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载角色失败: %1").arg(e.what()));
    }
}

void RoleManagementPage::addRole() {
    showRoleDialog();
}

void RoleManagementPage::editRole() {
    auto selected = table_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要编辑的角色");
        return;
    }
    
    int roleId = selected[0]->text().toInt();
    try {
        auto role = client_->getRole(roleId);
        showRoleDialog(&role);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("获取角色信息失败: %1").arg(e.what()));
    }
}

void RoleManagementPage::deleteRole() {
    auto selected = table_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要删除的角色");
        return;
    }
    
    int roleId = selected[0]->text().toInt();
    QString roleName = selected[1]->text();
    
    if (QMessageBox::question(this, "确认删除", 
        QString("确定要删除角色 \"%1\" 吗？").arg(roleName)) == QMessageBox::Yes) {
        try {
            client_->deleteRole(roleId);
            QMessageBox::information(this, "成功", "角色删除成功");
            loadRoles();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "错误", QString("删除角色失败: %1").arg(e.what()));
        }
    }
}

void RoleManagementPage::showRoleDialog(const admin::Role* role) {
    QDialog dialog(this);
    dialog.setWindowTitle(role ? "编辑角色" : "添加角色");
    dialog.resize(400, 250);

    auto* layout = new QFormLayout(&dialog);

    QLineEdit* nameEdit = new QLineEdit();
    QTextEdit* descEdit = new QTextEdit();
    QCheckBox* activeCheck = new QCheckBox("启用");
    activeCheck->setChecked(true);

    if (role) {
        nameEdit->setText(QString::fromStdString(role->name()));
        descEdit->setText(QString::fromStdString(role->description()));
        activeCheck->setChecked(role->active());
    }

    layout->addRow("角色名称:", nameEdit);
    layout->addRow("描述:", descEdit);
    layout->addRow(activeCheck);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            if (role) {
                client_->updateRole(
                    role->id(),
                    nameEdit->text().toStdString(),
                    descEdit->toPlainText().toStdString(),
                    activeCheck->isChecked()
                );
                QMessageBox::information(this, "成功", "角色信息更新成功");
            } else {
                client_->createRole(
                    nameEdit->text().toStdString(),
                    descEdit->toPlainText().toStdString()
                );
                QMessageBox::information(this, "成功", "角色创建成功");
            }
            loadRoles();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "错误", QString("操作失败: %1").arg(e.what()));
        }
    }
}
