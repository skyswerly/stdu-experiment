#include "ThreadWidget.h"
#include "Scheduler.h"
#include "SJFScheduler.h"
#include "HRRNScheduler.h"
#include "DynamicPriorityScheduler.h"
#include "MyThread.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>

#include <sstream>
#include <iomanip>

ThreadWidget::ThreadWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    onProcessCountChanged(5);  // 默认 5 个进程
}

ThreadWidget::~ThreadWidget() = default;

void ThreadWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // === 第1行：算法选择 + 进程数 ===
    auto* topGroup = new QGroupBox("调度配置", this);
    auto* topLayout = new QHBoxLayout(topGroup);

    topLayout->addWidget(new QLabel("调度算法:", this));
    m_algoCombo = new QComboBox(this);
    m_algoCombo->addItem("非抢占短作业优先 (SJF)");
    m_algoCombo->addItem("最高响应比优先 (HRRN)");
    m_algoCombo->addItem("动态优先级调度");
    topLayout->addWidget(m_algoCombo);

    topLayout->addSpacing(20);

    topLayout->addWidget(new QLabel("进程数:", this));
    m_processCountSpin = new QSpinBox(this);
    m_processCountSpin->setRange(1, 20);
    m_processCountSpin->setValue(5);
    topLayout->addWidget(m_processCountSpin);

    m_startBtn = new QPushButton("开始调度", this);
    m_startBtn->setMinimumWidth(120);
    topLayout->addWidget(m_startBtn);

    topLayout->addStretch();
    mainLayout->addWidget(topGroup);

    // === 第2行：进程参数表 ===
    auto* tableGroup = new QGroupBox("进程参数（双击编辑）", this);
    auto* tableLayout = new QVBoxLayout(tableGroup);
    m_processTable = new QTableWidget(this);
    m_processTable->setColumnCount(5);
    m_processTable->setHorizontalHeaderLabels(
        {"PID", "进程名", "CPU时间(ms)", "基础优先级", "到达时间(ms)"});
    m_processTable->horizontalHeader()->setStretchLastSection(true);
    m_processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableLayout->addWidget(m_processTable);
    mainLayout->addWidget(tableGroup, 1);

    // === 第3行：调度日志 + 结果 ===
    auto* bottomLayout = new QHBoxLayout();

    // 左侧：调度日志
    auto* logGroup = new QGroupBox("调度过程日志", this);
    auto* logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QPlainTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(1000);
    logLayout->addWidget(m_logEdit);
    bottomLayout->addWidget(logGroup, 2);

    // 右侧：结果表
    auto* resultGroup = new QGroupBox("调度结果汇总", this);
    auto* resultLayout = new QVBoxLayout(resultGroup);
    m_resultTable = new QTableWidget(this);
    m_resultTable->setColumnCount(5);
    m_resultTable->setHorizontalHeaderLabels(
        {"进程", "开始(ms)", "完成(ms)", "周转(ms)", "带权周转"});
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultLayout->addWidget(m_resultTable);

    // 统计标签
    m_avgTurnaroundLabel = new QLabel("平均周转时间: -- ms", this);
    m_avgWeightedLabel = new QLabel("平均带权周转时间: --", this);
    resultLayout->addWidget(m_avgTurnaroundLabel);
    resultLayout->addWidget(m_avgWeightedLabel);

    bottomLayout->addWidget(resultGroup, 1);
    mainLayout->addLayout(bottomLayout, 2);

    // === 信号连接 ===
    connect(m_processCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ThreadWidget::onProcessCountChanged);
    connect(m_startBtn, &QPushButton::clicked,
            this, &ThreadWidget::onStartSchedule);
}

void ThreadWidget::onProcessCountChanged(int count)
{
    // 调整进程参数表行数，填入示例数据
    m_processTable->setRowCount(count);

    // 示例数据
    struct SampleData { int pid; const char* name; int total; int pri; int arrive; };
    static const SampleData samples[] = {
        {1, "P1", 500, 10, 0},
        {2, "P2", 300, 5,  50},
        {3, "P3", 400, 8,  100},
        {4, "P4", 200, 3,  150},
        {5, "P5", 600, 12, 200},
    };

    for (int i = 0; i < count; ++i) {
        const auto& s = samples[i % 5];
        m_processTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.pid + (i / 5) * 5)));
        m_processTable->setItem(i, 1, new QTableWidgetItem(
            QString("P%1").arg(s.pid + (i / 5) * 5)));
        m_processTable->setItem(i, 2, new QTableWidgetItem(QString::number(s.total)));
        m_processTable->setItem(i, 3, new QTableWidgetItem(QString::number(s.pri)));
        m_processTable->setItem(i, 4, new QTableWidgetItem(QString::number(s.arrive)));
    }
}

void ThreadWidget::onStartSchedule()
{
    int choice = m_algoCombo->currentIndex();  // 0=SJF, 1=HRRN, 2=DynamicPriority
    int count = m_processCountSpin->value();

    // 读取进程参数
    struct ProcessParam {
        int pid;
        std::string name;
        int totalTime;
        int basePriority;
        int arriveTime;
    };
    std::vector<ProcessParam> params;

    for (int i = 0; i < count; ++i) {
        ProcessParam p;
        p.pid = m_processTable->item(i, 0)
                    ? m_processTable->item(i, 0)->text().toInt() : (i + 1);
        p.name = m_processTable->item(i, 1)
                     ? m_processTable->item(i, 1)->text().toStdString() : ("P" + std::to_string(i + 1));
        p.totalTime = m_processTable->item(i, 2)
                          ? m_processTable->item(i, 2)->text().toInt() : 100;
        p.basePriority = m_processTable->item(i, 3)
                             ? m_processTable->item(i, 3)->text().toInt() : 5;
        p.arriveTime = m_processTable->item(i, 4)
                           ? m_processTable->item(i, 4)->text().toInt() : 0;
        params.push_back(p);
    }

    // 禁用按钮防止重复点击
    m_startBtn->setEnabled(false);
    m_logEdit->clear();
    m_resultTable->setRowCount(0);
    m_lastResults.clear();

    // ===== 在主线程创建进程和调度器 =====
    // （std::thread 不可拷贝，用指针存储；对象的实际使用在后台线程，但生命周期由主线程管理）
    std::vector<MyThread*> processes;
    Scheduler* scheduler = nullptr;

    if (choice == 0)
        scheduler = new SJFScheduler();
    else if (choice == 1)
        scheduler = new HRRNScheduler();
    else
        scheduler = new DynamicPriorityScheduler();

    for (const auto& p : params) {
        MyThread* t = new MyThread(
            p.pid, p.name, p.totalTime, p.basePriority, p.arriveTime);
        processes.push_back(t);
        scheduler->addProcess(t);
    }

    // ===== 后台线程执行调度，完成后主线程提取结果 =====
    auto* watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this,
            [this, watcher, processes, scheduler]() {
                // 后台线程已完成，安全读取进程数据
                QString log = watcher->result();
                m_logEdit->setPlainText(log);

                // 从进程对象提取结果数据
                m_lastResults.clear();
                for (auto* p : processes) {
                    ProcessResult r;
                    r.name = QString::fromStdString(p->getName());
                    r.startTime = p->getStartTime();
                    r.finishTime = p->getFinishTime();
                    r.turnaround = p->calculateTurnaroundTime();
                    r.weightedTurnaround = p->calculateWeightedTurnaroundTime();
                    m_lastResults.push_back(r);
                }

                populateResultTable();

                // 清理
                for (auto* p : processes) delete p;
                delete scheduler;
                watcher->deleteLater();
            });

    // 后台线程：执行调度 + 捕获 cout 输出
    QFuture<QString> future = QtConcurrent::run(
        [processes, scheduler]() -> QString {
            std::ostringstream oss;
            auto* oldBuf = std::cout.rdbuf(oss.rdbuf());

            scheduler->schedule();
            scheduler->displayResults();

            std::cout.rdbuf(oldBuf);
            return QString::fromStdString(oss.str());
        });

    watcher->setFuture(future);
}

void ThreadWidget::onScheduleFinished()
{
    // 所有工作在 connect lambda 中完成（提取结果 + 填充表格 + 清理）
    // 此槽函数保留以备其他用途
}

void ThreadWidget::populateResultTable()
{
    if (m_lastResults.empty()) return;

    m_resultTable->setRowCount(static_cast<int>(m_lastResults.size()));

    double totalTurnaround = 0.0;
    double totalWeighted = 0.0;

    for (size_t i = 0; i < m_lastResults.size(); ++i) {
        const auto& r = m_lastResults[i];
        int row = static_cast<int>(i);

        m_resultTable->setItem(row, 0, new QTableWidgetItem(r.name));
        m_resultTable->setItem(row, 1, new QTableWidgetItem(
            r.startTime >= 0 ? QString::number(r.startTime) : "-"));
        m_resultTable->setItem(row, 2, new QTableWidgetItem(
            r.finishTime >= 0 ? QString::number(r.finishTime) : "-"));
        m_resultTable->setItem(row, 3, new QTableWidgetItem(
            r.turnaround >= 0 ? QString::number(r.turnaround) : "-"));
        m_resultTable->setItem(row, 4, new QTableWidgetItem(
            r.weightedTurnaround >= 0.0
                ? QString::number(r.weightedTurnaround, 'f', 2) : "-"));

        if (r.turnaround >= 0) {
            totalTurnaround += r.turnaround;
            totalWeighted += r.weightedTurnaround;
        }
    }

    int validCount = 0;
    for (const auto& r : m_lastResults) {
        if (r.turnaround >= 0) ++validCount;
    }

    if (validCount > 0) {
        double avgTurnaround = totalTurnaround / validCount;
        double avgWeighted = totalWeighted / validCount;
        m_avgTurnaroundLabel->setText(
            QString("平均周转时间: %1 ms").arg(avgTurnaround, 0, 'f', 2));
        m_avgWeightedLabel->setText(
            QString("平均带权周转时间: %1").arg(avgWeighted, 0, 'f', 2));
    }

    m_startBtn->setEnabled(true);
}
