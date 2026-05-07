#pragma once
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QVBarModelMapper>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "bi.grpc.pb.h"

class BIClient;

class BIReportPage : public QWidget {
    Q_OBJECT
public:
    explicit BIReportPage(BIClient* client, QWidget* parent = nullptr);

private slots:
    void onGenerateReport();
    void onExportReport();
    void onRefreshReports();
    void onReportSelected(int row);
    void onAddSchedule();
    void onRefreshSchedules();

private:
    void setupUi();
    void loadReports();
    void loadSchedules();
    void displayReport(const bi::ReportResult& report);

    BIClient* client_;
    
    // Report generation UI
    QComboBox* reportTypeCombo_;
    QDateEdit* startDateEdit_;
    QDateEdit* endDateEdit_;
    QPushButton* generateBtn_;
    QPushButton* exportBtn_;
    
    // Reports table
    QTableWidget* reportsTable_;
    
    // Charts
    QChartView* chartView_;
    QLabel* reportTitleLabel_;
    QTableWidget* dataTable_;
    
    // Scheduler UI
    QTableWidget* schedulesTable_;
    QPushButton* addScheduleBtn_;
    
    QString currentReportId_;
};