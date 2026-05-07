#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>
#include <QTimer>

#include "trajectory_view.h"
#include "network/trajectory_client.h"

class TracePage : public QWidget {
    Q_OBJECT
    
public:
    explicit TracePage(TrajectoryClient* client, QWidget* parent = nullptr);
    ~TracePage();
    
private slots:
    void onSearch();
    void onPlay();
    void onStop();
    void onReset();
    void onAnimationFinished();
    void onPointReached(const trajectory::TrajectoryPoint& point);
    void updateTimeAxis();
    
private:
    void updateStatus(const QString& status);
    void updateAssetInfo(int assetId, const QString& name);
    
    TrajectoryClient* client;
    TrajectoryView* trajectoryView;
    
    QLineEdit* assetIdEdit;
    QPushButton* searchBtn;
    QPushButton* playBtn;
    QPushButton* stopBtn;
    QPushButton* resetBtn;
    
    QLabel* statusLabel;
    QLabel* assetNameLabel;
    QLabel* currentLocationLabel;
    QLabel* riskLabel;
    QProgressBar* progressBar;
    
    QTimer* timeAxisTimer;
    std::vector<trajectory::TrajectoryPoint> currentTrajectory;
    int currentPointIndex;
};