#ifndef THREAD_WIDGET_H
#define THREAD_WIDGET_H

#include <QWidget>
#include <vector>
#include <memory>

class QComboBox;
class QSpinBox;
class QTableWidget;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class MyThread;
class Scheduler;

struct ProcessResult
{
    QString name;
    int startTime = 0;
    int finishTime = 0;
    int turnaround = 0;
    double weightedTurnaround = 0.0;
};

class ThreadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThreadWidget(QWidget* parent = nullptr);
    ~ThreadWidget() override;

private slots:
    void onProcessCountChanged(int count);
    void onStartSchedule();
    void onScheduleFinished();

private:
    void setupUI();
    void populateResultTable();

    // 算法选择
    QComboBox* m_algoCombo;
    QSpinBox* m_processCountSpin;

    // 进程参数表（可编辑）
    QTableWidget* m_processTable;

    // 操作按钮
    QPushButton* m_startBtn;

    // 调度过程日志
    QPlainTextEdit* m_logEdit;

    // 结果汇总表
    QTableWidget* m_resultTable;

    // 汇总统计标签
    QLabel* m_avgTurnaroundLabel;
    QLabel* m_avgWeightedLabel;

    // 上一次调度的结果数据
    std::vector<ProcessResult> m_lastResults;
};

#endif // THREAD_WIDGET_H
