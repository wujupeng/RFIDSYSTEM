#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include "../../network/admin_client.h"

class SystemSettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SystemSettingsPage(AdminClient* client, QWidget* parent = nullptr);

private slots:
    void loadSettings();
    void saveSettings();

private:
    AdminClient* client_;
    QLineEdit* systemNameEdit_;
    QLabel* versionLabel_;
    QSpinBox* sessionTimeoutSpin_;
    QCheckBox* auditLogCheck_;
    QCheckBox* notificationsCheck_;
    QPushButton* saveBtn_;
};
