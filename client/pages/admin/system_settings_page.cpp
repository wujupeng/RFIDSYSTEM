#include "system_settings_page.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGroupBox>

SystemSettingsPage::SystemSettingsPage(AdminClient* client, QWidget* parent)
    : QWidget(parent), client_(client)
{
    auto* mainLayout = new QVBoxLayout(this);

    QGroupBox* generalGroup = new QGroupBox("系统信息");
    auto* generalLayout = new QFormLayout();

    systemNameEdit_ = new QLineEdit();
    versionLabel_ = new QLabel();
    versionLabel_->setStyleSheet("color: #666; font-style: italic;");

    generalLayout->addRow("系统名称:", systemNameEdit_);
    generalLayout->addRow("系统版本:", versionLabel_);
    generalGroup->setLayout(generalLayout);
    mainLayout->addWidget(generalGroup);

    QGroupBox* securityGroup = new QGroupBox("安全设置");
    auto* securityLayout = new QFormLayout();

    sessionTimeoutSpin_ = new QSpinBox();
    sessionTimeoutSpin_->setRange(5, 120);
    sessionTimeoutSpin_->setSuffix(" 分钟");

    securityLayout->addRow("会话超时时间:", sessionTimeoutSpin_);
    securityGroup->setLayout(securityLayout);
    mainLayout->addWidget(securityGroup);

    QGroupBox* featureGroup = new QGroupBox("功能开关");
    auto* featureLayout = new QFormLayout();

    auditLogCheck_ = new QCheckBox("启用审计日志");
    notificationsCheck_ = new QCheckBox("启用系统通知");

    featureLayout->addRow(auditLogCheck_);
    featureLayout->addRow(notificationsCheck_);
    featureGroup->setLayout(featureLayout);
    mainLayout->addWidget(featureGroup);

    saveBtn_ = new QPushButton("保存设置");
    connect(saveBtn_, &QPushButton::clicked, this, &SystemSettingsPage::saveSettings);
    mainLayout->addWidget(saveBtn_);

    mainLayout->addStretch();

    loadSettings();
}

void SystemSettingsPage::loadSettings() {
    try {
        auto settings = client_->getSystemSettings();
        
        systemNameEdit_->setText(QString::fromStdString(settings.system_name()));
        versionLabel_->setText(QString::fromStdString(settings.system_version()));
        sessionTimeoutSpin_->setValue(settings.session_timeout_minutes());
        auditLogCheck_->setChecked(settings.enable_audit_log());
        notificationsCheck_->setChecked(settings.enable_notifications());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载设置失败: %1").arg(e.what()));
    }
}

void SystemSettingsPage::saveSettings() {
    try {
        client_->updateSystemSettings(
            systemNameEdit_->text().toStdString(),
            sessionTimeoutSpin_->value(),
            auditLogCheck_->isChecked(),
            notificationsCheck_->isChecked()
        );
        QMessageBox::information(this, "成功", "系统设置保存成功");
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("保存设置失败: %1").arg(e.what()));
    }
}
