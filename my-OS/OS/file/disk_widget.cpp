#include "disk_widget.h"
#include "disk_scheduler.h"
#include "algorithm_compare.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <ctime>

DiskWidget::DiskWidget(QWidget* parent)
    : QWidget(parent) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    setupUI();
}

void DiskWidget::setupUI() {
    auto* main_layout = new QVBoxLayout(this);

    // ====== 参数设置 ======
    auto* param_group = new QGroupBox("参数设置", this);
    auto* param_layout = new QHBoxLayout(param_group);

    param_layout->addWidget(new QLabel("起始磁道号:", this));
    start_track_spin_ = new QSpinBox(this);
    start_track_spin_->setRange(0, 65535);
    start_track_spin_->setValue(100);
    param_layout->addWidget(start_track_spin_);

    param_layout->addWidget(new QLabel("最大磁道号:", this));
    max_track_spin_ = new QSpinBox(this);
    max_track_spin_->setRange(1, 65535);
    max_track_spin_->setValue(200);
    param_layout->addWidget(max_track_spin_);

    param_layout->addWidget(new QLabel("请求数量:", this));
    request_count_spin_ = new QSpinBox(this);
    request_count_spin_->setRange(1, 100);
    request_count_spin_->setValue(10);
    param_layout->addWidget(request_count_spin_);

    param_layout->addWidget(new QLabel("算法:", this));
    algorithm_combo_ = new QComboBox(this);
    algorithm_combo_->addItems({"FCFS", "SSTF", "SCAN", "CSCAN", "N-Step-SCAN"});
    param_layout->addWidget(algorithm_combo_);

    execute_btn_ = new QPushButton("执行", this);
    param_layout->addWidget(execute_btn_);

    compare_btn_ = new QPushButton("对比全部", this);
    param_layout->addWidget(compare_btn_);

    main_layout->addWidget(param_group);

    // ====== 寻道结果表格 ======
    result_table_ = new QTableWidget(this);
    result_table_->setColumnCount(3);
    result_table_->setHorizontalHeaderLabels({"步数", "磁道号", "移动距离"});
    result_table_->horizontalHeader()->setStretchLastSection(true);
    result_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    main_layout->addWidget(result_table_);

    // ====== 统计信息 ======
    auto* stats_layout = new QHBoxLayout();
    direction_label_ = new QLabel("方向: --", this);
    total_label_     = new QLabel("总距离: --", this);
    avg_label_       = new QLabel("平均寻道长度: --", this);
    stats_layout->addWidget(direction_label_);
    stats_layout->addWidget(total_label_);
    stats_layout->addWidget(avg_label_);
    stats_layout->addStretch();
    main_layout->addLayout(stats_layout);

    // ====== 算法对比表格 ======
    compare_table_ = new QTableWidget(this);
    compare_table_->setColumnCount(4);
    compare_table_->setHorizontalHeaderLabels({"排名", "算法", "总距离", "平均长度"});
    compare_table_->horizontalHeader()->setStretchLastSection(true);
    compare_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compare_table_->setMaximumHeight(200);
    main_layout->addWidget(compare_table_);

    // 连接信号
    connect(execute_btn_, &QPushButton::clicked, this, &DiskWidget::onExecute);
    connect(compare_btn_, &QPushButton::clicked, this, &DiskWidget::onCompareAll);
}

void DiskWidget::onExecute() {
    int start   = start_track_spin_->value();
    int max_trk = max_track_spin_->value();
    int count   = request_count_spin_->value();
    int algo    = algorithm_combo_->currentIndex();

    scheduler_.generateRequests(count, max_trk);

    DiskResult result;
    switch (algo) {
        case 0: result = scheduler_.FCFS(start);            break;
        case 1: result = scheduler_.SSTF(start);             break;
        case 2: result = scheduler_.SCAN(start, max_trk);    break;
        case 3: result = scheduler_.CSCAN(start, max_trk);   break;
        case 4: result = scheduler_.NStepSCAN(start, 5);     break;
    }

    // 填充寻道顺序表
    result_table_->setRowCount(static_cast<int>(result.seek_sequence.size()));
    int prev = start;
    for (int i = 0; i < static_cast<int>(result.seek_sequence.size()); ++i) {
        int track = result.seek_sequence[i];
        int dist = std::abs(track - prev);
        result_table_->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        result_table_->setItem(i, 1, new QTableWidgetItem(QString::number(track)));
        result_table_->setItem(i, 2, new QTableWidgetItem(QString::number(dist)));
        prev = track;
    }

    // 更新统计
    direction_label_->setText(QString("方向: %1").arg(result.direction ? "向外" : "向内"));
    total_label_->setText(QString("总距离: %1").arg(result.total_distance));
    avg_label_->setText(QString("平均寻道长度: %1").arg(result.avg_distance, 0, 'f', 2));
}

void DiskWidget::onCompareAll() {
    int start   = start_track_spin_->value();
    int max_trk = max_track_spin_->value();
    int count   = request_count_spin_->value();

    scheduler_.generateRequests(count, max_trk);

    auto entries = compareAll(scheduler_, start, max_trk);

    compare_table_->setRowCount(static_cast<int>(entries.size()));
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        compare_table_->setItem(i, 0, new QTableWidgetItem(QString::number(entries[i].rank)));
        compare_table_->setItem(i, 1, new QTableWidgetItem(entries[i].name));
        compare_table_->setItem(i, 2, new QTableWidgetItem(QString::number(entries[i].total_distance)));
        compare_table_->setItem(i, 3, new QTableWidgetItem(
            QString::number(entries[i].avg_distance, 'f', 2)));
    }

    // 同时执行当前选中的算法展示在上方
    onExecute();
}
