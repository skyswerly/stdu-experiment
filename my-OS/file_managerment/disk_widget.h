#pragma once

#include <QWidget>
#include "disk_scheduler.h"

class QSpinBox;
class QComboBox;
class QPushButton;
class QTableWidget;
class QLabel;

class DiskWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiskWidget(QWidget* parent = nullptr);

private slots:
    void onExecute();
    void onCompareAll();

private:
    void setupUI();

    QSpinBox* start_track_spin_;
    QSpinBox* max_track_spin_;
    QSpinBox* request_count_spin_;
    QComboBox* algorithm_combo_;
    QPushButton* execute_btn_;
    QPushButton* compare_btn_;
    QTableWidget* result_table_;
    QLabel* direction_label_;
    QLabel* total_label_;
    QLabel* avg_label_;
    QTableWidget* compare_table_;

    DiskScheduler scheduler_;
};
