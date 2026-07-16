#include "MemoryWidget.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>

// 构造函数
MemoryWidget::MemoryWidget(QWidget* parent)
    : QWidget(parent)
{
    resize(960, 720);
    setMinimumSize(800, 600);

    // UI布局
    setupUI();

    // 程序启动：生成 20 条指令地址并刷新地址表格
    m_addressGen.autoGenerate(20);
    refreshAddressTable();

    // 初始化页表
    m_pageTable.init(3);//默认页框数量为3
}

MemoryWidget::~MemoryWidget() = default;


// UI布局函数
void MemoryWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    //1. 参数配置区
    auto* paramGroup = new QGroupBox("参数配置", this);
    auto* paramLayout = new QHBoxLayout(paramGroup);

    paramLayout->addWidget(new QLabel("页面尺寸（字节）:", this));
    m_pageSizeSpin = new QSpinBox(this);
    m_pageSizeSpin->setRange(1024, 3072);
    m_pageSizeSpin->setSingleStep(1024);
    m_pageSizeSpin->setValue(1024);
    m_pageSizeSpin->setSuffix(" 字节");
    paramLayout->addWidget(m_pageSizeSpin);

    paramLayout->addSpacing(20);

    paramLayout->addWidget(new QLabel("页表长度（页框数）:", this));
    m_frameCountSpin = new QSpinBox(this);
    m_frameCountSpin->setRange(2, 4);
    m_frameCountSpin->setValue(3);
    m_frameCountSpin->setSuffix(" 个");
    paramLayout->addWidget(m_frameCountSpin);

    paramLayout->addSpacing(20);

    m_applyBtn = new QPushButton("应用参数", this);
    paramLayout->addWidget(m_applyBtn);

    paramLayout->addStretch();
    mainLayout->addWidget(paramGroup);

    //2. 算法操作区
    auto* algoGroup = new QGroupBox("算法选择与操作", this);
    auto* algoLayout = new QHBoxLayout(algoGroup);

    algoLayout->addWidget(new QLabel("置换算法:", this));
    m_fifoRadio = new QRadioButton("FIFO 算法", this);
    m_lruRadio  = new QRadioButton("LRU 算法", this);
    m_fifoRadio->setChecked(true);
    algoLayout->addWidget(m_fifoRadio);
    algoLayout->addWidget(m_lruRadio);

    algoLayout->addSpacing(30);

    m_startBtn = new QPushButton("开始模拟", this);
    m_clearBtn = new QPushButton("清空输出表", this);
    m_startBtn->setMinimumWidth(100);
    m_clearBtn->setMinimumWidth(100);
    algoLayout->addWidget(m_startBtn);
    algoLayout->addWidget(m_clearBtn);

    algoLayout->addStretch();
    mainLayout->addWidget(algoGroup);

    //3. 地址流 + 页表状态
    auto* midLayout = new QHBoxLayout();
    midLayout->setSpacing(10);

    //左侧：地址流
    auto* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel("逻辑地址 / 页号", this));
    m_addressTable = new QTableWidget(20, 2, this);
    m_addressTable->setHorizontalHeaderLabels({"逻辑地址", "页号"});
    m_addressTable->horizontalHeader()->setStretchLastSection(true);
    m_addressTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_addressTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_addressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_addressTable->verticalHeader()->setVisible(false);
    m_addressTable->setMaximumWidth(280);
    leftLayout->addWidget(m_addressTable);
    midLayout->addLayout(leftLayout);

    //右侧：页表状态
    auto* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel("页表输出（每次访问后的页框状态）", this));
    m_outputTable = new QTableWidget(20, 3, this);  // 默认 3 列，动态调整
    m_outputTable->setHorizontalHeaderLabels({"页框0", "页框1", "页框2"});
    m_outputTable->horizontalHeader()->setStretchLastSection(true);
    m_outputTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_outputTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_outputTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 设置行号标签宽度
    m_outputTable->verticalHeader()->setVisible(true);
    rightLayout->addWidget(m_outputTable);
    midLayout->addLayout(rightLayout);

    mainLayout->addLayout(midLayout, 1);  // stretch=1：占据主要空间

    //4. 底部统计区
    auto* statsLayout = new QHBoxLayout();
    m_hitCountLabel = new QLabel("命中次数: 0", this);
    m_hitRateLabel  = new QLabel("命中率: 0.0%", this);
    QFont statsFont = m_hitCountLabel->font();
    statsFont.setPointSize(12);
    statsFont.setBold(true);
    m_hitCountLabel->setFont(statsFont);
    m_hitRateLabel->setFont(statsFont);
    statsLayout->addWidget(m_hitCountLabel);
    statsLayout->addSpacing(40);
    statsLayout->addWidget(m_hitRateLabel);
    statsLayout->addStretch();
    mainLayout->addLayout(statsLayout);

    //信号 / 槽连接
    connect(m_applyBtn,  &QPushButton::clicked, this, &MemoryWidget::onApplyParams);
    connect(m_startBtn,  &QPushButton::clicked, this, &MemoryWidget::onStartSimulation);
    connect(m_clearBtn,  &QPushButton::clicked, this, &MemoryWidget::onClearTable);

    // 算法切换
    connect(m_fifoRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) onAlgorithmChanged();
    });
    connect(m_lruRadio,  &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) onAlgorithmChanged();
    });
}

// 槽函数：应用参数
void MemoryWidget::onApplyParams() {
    int pageSize   = m_pageSizeSpin->value();
    int frameCount = m_frameCountSpin->value();

    // 参数校验
    if (pageSize < 1024 || pageSize > 3072) {
        QMessageBox::warning(this, "参数错误",
                             "页面尺寸必须在 1024~3072 字节之间！已重置为默认值 1024。");
        m_pageSizeSpin->setValue(1024);
        pageSize = 1024;
    }
    if (frameCount < 2 || frameCount > 4) {
        QMessageBox::warning(this, "参数错误",
                             "页表长度必须在 2~4 之间！已重置为默认值 3。");
        m_frameCountSpin->setValue(3);
        frameCount = 3;
    }

    // 重新生成地址流
    m_addressGen.autoGenerate(20);

    // 刷新地址表格
    refreshAddressTable();

    // 更新输出表格列数
    m_outputTable->setColumnCount(frameCount);
    QStringList headers;
    for (int i = 0; i < frameCount; i++) {
        headers << QString("页框%1").arg(i);
    }
    m_outputTable->setHorizontalHeaderLabels(headers);

    // 初始化/重置页表
    m_pageTable.init(frameCount);

    // 重置模拟状态
    resetSimulation();
}

// 槽函数：算法切换
void MemoryWidget::onAlgorithmChanged() {
    if (m_fifoRadio->isChecked()) {
        m_currentReplacer = &m_fifoReplacer;
    } else {
        m_currentReplacer = &m_lruReplacer;
    }

    // 切换算法后自动重置
    m_pageTable.init(m_frameCountSpin->value());
    m_fifoReplacer.reset();
    m_lruReplacer.reset();
    resetSimulation();
}

// 槽函数：开始模拟
void MemoryWidget::onStartSimulation() {
    int pageSize   = m_pageSizeSpin->value();
    int frameCount = m_frameCountSpin->value();
    const auto& addresses = m_addressGen.getAddresses();

    if (addresses.empty()) {
        QMessageBox::warning(this, "无数据", "请先生成地址流！");
        return;
    }

    // 重置准备开始
    m_pageTable.init(frameCount);
    m_currentReplacer->reset();
    m_outputTable->clearContents();
    // 确保输出表格行数足够
    m_outputTable->setRowCount(static_cast<int>(addresses.size()));

    int hitCount = 0;
    int total    = static_cast<int>(addresses.size());

    // 逐条访问每条逻辑地址
    for (int step = 0; step < total; step++) {
        int addr   = addresses[step];
        int pageId = addr / pageSize;

        // 执行访问
        PageTableManager::AccessResult result =
            m_pageTable.access(pageId, m_currentReplacer);

        if (result.hit) {
            hitCount++;
        }

        // 将当前页框状态写入输出表格第 step 行
        std::vector<int> snapshot = m_pageTable.getFrameSnapshot();
        for (int col = 0; col < frameCount; col++) {
            QString cellText;
            if (col < static_cast<int>(snapshot.size()) && snapshot[col] != -1) {
                cellText = QString::number(snapshot[col]);
            } else {
                cellText = "空";
            }
            m_outputTable->setItem(step, col, new QTableWidgetItem(cellText));
        }
    }

    // 更新统计显示
    double hitRate = (total > 0) ? (100.0 * hitCount / total) : 0.0;
    m_hitCountLabel->setText(QString("命中次数: %1").arg(hitCount));
    m_hitRateLabel->setText(QString("命中率: %1%").arg(hitRate, 0, 'f', 1));

    // 禁用开始按钮，直到切换算法或清空
    m_startBtn->setEnabled(false);
}

// 槽函数：清空输出表
void MemoryWidget::onClearTable() {
    m_pageTable.init(m_frameCountSpin->value());
    m_currentReplacer->reset();
    resetSimulation();
}

// 辅助函数：刷新地址表格
void MemoryWidget::refreshAddressTable() {
    const auto& addresses = m_addressGen.getAddresses();
    int pageSize = m_pageSizeSpin->value();
    auto pages   = m_addressGen.getPages(pageSize);

    m_addressTable->clearContents();
    m_addressTable->setRowCount(static_cast<int>(addresses.size()));

    for (size_t i = 0; i < addresses.size(); i++) {
        m_addressTable->setItem(static_cast<int>(i), 0,
            new QTableWidgetItem(QString::number(addresses[i])));
        m_addressTable->setItem(static_cast<int>(i), 1,
            new QTableWidgetItem(QString::number(pages[i])));
    }
}

void MemoryWidget::resetSimulation() {
    // 清空输出表格内容
    m_outputTable->clearContents();
    // 重置统计标签
    m_hitCountLabel->setText("命中次数: 0");
    m_hitRateLabel->setText("命中率: 0.0%");
    // 恢复开始按钮
    m_startBtn->setEnabled(true);
}
