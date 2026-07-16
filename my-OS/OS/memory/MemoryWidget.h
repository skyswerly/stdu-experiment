#ifndef MEMORY_WIDGET_H
#define MEMORY_WIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

#include "address_generator.h"
#include "page_table_manager.h"
#include "i_replacer.h"
#include "fifo_replacer.h"
#include "lru_replacer.h"

// 页面置换模拟可视化界面
class MemoryWidget : public QWidget {
    Q_OBJECT  // 宏声明可使用信号槽机制

public:
    explicit MemoryWidget(QWidget* parent = nullptr);  // explicit 避免隐式类型转换
    ~MemoryWidget() override;

private slots:  // 私有槽函数
    // 应用参数：校验输入、重新生成地址、刷新地址表、重置输出
    void onApplyParams();

    // 算法切换：更换 replacer、重置页表和输出
    void onAlgorithmChanged();

    // 开始模拟：逐条执行 20 条地址访问，刷新输出表格和统计
    void onStartSimulation();

    // 清空输出表：清空输出表格、重置页表、清零统计
    void onClearTable();

private:
    // 构建 UI 布局
    void setupUI();

    // 刷新左侧地址流表格（20 行 × 2 列）
    void refreshAddressTable();

    // 重置模拟状态：清空页表输出表格、重置页表管理器、复位两个 Replacer、清零统计、启用开始按钮
    void resetSimulation();

    //======业务对象======
    AddressGenerator  m_addressGen;     // 地址生成器
    PageTableManager  m_pageTable;      // 页表管理器
    FifoReplacer      m_fifoReplacer;   // FIFO 算法实例
    LruReplacer       m_lruReplacer;    // LRU 算法实例
    IReplacer*        m_currentReplacer = &m_fifoReplacer;  // 当前算法指针

    //======参数设置控件======
    QSpinBox*    m_pageSizeSpin;    // 页面尺寸（1024~3072，步长 1024）
    QSpinBox*    m_frameCountSpin;  // 页表长度（2~4）
    QPushButton* m_applyBtn;       // 应用参数按钮

    //======算法操作控件======
    QRadioButton* m_fifoRadio;     // FIFO 算法
    QRadioButton* m_lruRadio;      // LRU 算法
    QPushButton*  m_startBtn;      // 开始模拟
    QPushButton*  m_clearBtn;      // 清空输出表

    //======地址/页表显示======
    QTableWidget* m_addressTable;   // 逻辑地址 + 页号（20 行）
    QTableWidget* m_outputTable;    // 20 行对应 20 条指令执行后页表的状态

    //======统计标签======
    QLabel* m_hitCountLabel;   // 命中次数
    QLabel* m_hitRateLabel;    // 命中率
};

#endif // MEMORY_WIDGET_H
