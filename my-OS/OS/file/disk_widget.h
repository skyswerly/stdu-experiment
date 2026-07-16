#ifndef DISK_WIDGET_H
#define DISK_WIDGET_H

#include <QWidget>
#include "disk_scheduler.h"

class QSpinBox;
class QComboBox;
class QPushButton;
class QTableWidget;
class QLabel;

// 磁盘调度可视化界面
class DiskWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiskWidget(QWidget* parent = nullptr);

private slots:
    // 执行当前选择的算法
    void onExecute();
    // 对比全部 5 种算法
    void onCompareAll();

private:
    void setupUI();

    //======参数控件======
    QSpinBox* start_track_spin_;     // 起始磁道号
    QSpinBox* max_track_spin_;       // 最大磁道号
    QSpinBox* request_count_spin_;   // 请求数量
    QComboBox* algorithm_combo_;     // 算法选择
    QPushButton* execute_btn_;       // 执行按钮
    QPushButton* compare_btn_;       // 对比全部按钮

    //======结果显示======
    QTableWidget* result_table_;     // 寻道结果表格
    QLabel* direction_label_;        // 方向
    QLabel* total_label_;            // 总距离
    QLabel* avg_label_;              // 平均寻道长度
    QTableWidget* compare_table_;    // 算法对比表格

    DiskScheduler scheduler_;        // 磁盘调度器
};

#endif // DISK_WIDGET_H
