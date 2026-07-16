#include "DeviceWidget.h"
#include "banker_algorithm.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <sstream>

// 默认示例数据（与原 console 版一致）
static const std::vector<std::vector<int>> DEFAULT_MAX = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};
static const std::vector<int> DEFAULT_AVAILABLE = {10, 5, 7};

DeviceWidget::DeviceWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    onInitialize();  // 启动时用默认数据初始化
}

DeviceWidget::~DeviceWidget() = default;

void DeviceWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // === 第1行：初始化参数 ===
    auto* initGroup = new QGroupBox("系统初始化", this);
    auto* initLayout = new QHBoxLayout(initGroup);

    initLayout->addWidget(new QLabel("进程数:", this));
    m_processCountSpin = new QSpinBox(this);
    m_processCountSpin->setRange(1, 20);
    m_processCountSpin->setValue(5);
    initLayout->addWidget(m_processCountSpin);

    initLayout->addWidget(new QLabel("资源类型数:", this));
    m_resourceCountSpin = new QSpinBox(this);
    m_resourceCountSpin->setRange(1, 10);
    m_resourceCountSpin->setValue(3);
    initLayout->addWidget(m_resourceCountSpin);

    m_initBtn = new QPushButton("初始化 / 重置", this);
    initLayout->addWidget(m_initBtn);

    initLayout->addStretch();
    mainLayout->addWidget(initGroup);

    // === 第2行：可用资源 + MAX 矩阵 ===
    auto* resGroup = new QGroupBox("资源配置", this);
    auto* resLayout = new QVBoxLayout(resGroup);

    // 可用资源
    auto* availLayout = new QHBoxLayout();
    m_availableLabel = new QLabel("可用资源 (空格分隔):", this);
    availLayout->addWidget(m_availableLabel);
    m_availableInput = new QLineEdit("10 5 7", this);
    availLayout->addWidget(m_availableInput, 1);
    resLayout->addLayout(availLayout);

    // MAX 矩阵
    m_maxLabel = new QLabel("最大需求矩阵 MAX (双击编辑):", this);
    resLayout->addWidget(m_maxLabel);
    m_maxTable = new QTableWidget(this);
    m_maxTable->horizontalHeader()->setStretchLastSection(true);
    resLayout->addWidget(m_maxTable);

    mainLayout->addWidget(resGroup);

    // === 第3行：资源申请 ===
    auto* reqGroup = new QGroupBox("资源申请", this);
    auto* reqLayout = new QHBoxLayout(reqGroup);

    reqLayout->addWidget(new QLabel("进程号:", this));
    m_requestPidSpin = new QSpinBox(this);
    m_requestPidSpin->setRange(0, 19);
    reqLayout->addWidget(m_requestPidSpin);

    reqLayout->addWidget(new QLabel("申请资源 (空格分隔):", this));
    m_requestInput = new QLineEdit(this);
    reqLayout->addWidget(m_requestInput, 1);

    m_requestBtn = new QPushButton("申请资源", this);
    reqLayout->addWidget(m_requestBtn);

    mainLayout->addWidget(reqGroup);

    // === 第4行：状态矩阵（ALLOCATION + NEED）===
    auto* statusLayout = new QHBoxLayout();

    auto* allocLayout = new QVBoxLayout();
    allocLayout->addWidget(new QLabel("已分配矩阵 ALLOCATION:", this));
    m_allocTable = new QTableWidget(this);
    m_allocTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_allocTable->horizontalHeader()->setStretchLastSection(true);
    allocLayout->addWidget(m_allocTable);
    statusLayout->addLayout(allocLayout);

    auto* needLayout = new QVBoxLayout();
    needLayout->addWidget(new QLabel("需求矩阵 NEED:", this));
    m_needTable = new QTableWidget(this);
    m_needTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_needTable->horizontalHeader()->setStretchLastSection(true);
    needLayout->addWidget(m_needTable);
    statusLayout->addLayout(needLayout);

    mainLayout->addLayout(statusLayout, 1);

    // === 第5行：日志 ===
    auto* logGroup = new QGroupBox("操作日志", this);
    auto* logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QPlainTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(500);
    logLayout->addWidget(m_logEdit);
    mainLayout->addWidget(logGroup);

    // === 信号连接 ===
    connect(m_initBtn, &QPushButton::clicked, this, &DeviceWidget::onInitialize);
    connect(m_requestBtn, &QPushButton::clicked, this, &DeviceWidget::onRequestResource);
}

void DeviceWidget::onInitialize()
{
    m_processCount = m_processCountSpin->value();
    m_resourceCount = m_resourceCountSpin->value();

    // 调整 MAX 表格大小
    m_maxTable->setRowCount(m_processCount);
    m_maxTable->setColumnCount(m_resourceCount);
    QStringList headers;
    for (int j = 0; j < m_resourceCount; ++j)
        headers << QString("资源%1").arg(j);
    m_maxTable->setHorizontalHeaderLabels(headers);
    QStringList rowHeaders;
    for (int i = 0; i < m_processCount; ++i)
        rowHeaders << QString("P%1").arg(i);
    m_maxTable->setVerticalHeaderLabels(rowHeaders);

    // 填入默认 MAX 矩阵（如果行列匹配）
    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            int val = 0;
            if (i < static_cast<int>(DEFAULT_MAX.size())
                && j < static_cast<int>(DEFAULT_MAX[i].size())) {
                val = DEFAULT_MAX[i][j];
            }
            m_maxTable->setItem(i, j, new QTableWidgetItem(QString::number(val)));
        }
    }

    // 设置可用资源默认值
    QStringList availParts;
    for (int j = 0; j < m_resourceCount; ++j) {
        int val = (j < static_cast<int>(DEFAULT_AVAILABLE.size()))
                      ? DEFAULT_AVAILABLE[j] : 0;
        availParts << QString::number(val);
    }
    m_availableInput->setText(availParts.join(" "));

    // 读取 MAX 矩阵
    std::vector<std::vector<int>> max(m_processCount,
                                       std::vector<int>(m_resourceCount, 0));
    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            auto* item = m_maxTable->item(i, j);
            if (item) max[i][j] = item->text().toInt();
        }
    }

    // 读取可用资源
    std::vector<int> available = parseResourceInput(
        m_availableInput->text(), m_resourceCount);

    // 创建 BankerAlgorithm 实例
    try {
        m_banker = std::make_unique<BankerAlgorithm>(max, available);
        log("系统初始化成功！");
        log(QString("进程数 M = %1，资源类型数 N = %2")
                .arg(m_processCount).arg(m_resourceCount));
        log(QString("初始可用资源: %1")
                .arg(m_availableInput->text()));

        refreshStatusDisplay();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "初始化失败",
                             QString("BankerAlgorithm 构造异常: %1").arg(e.what()));
    }
}

void DeviceWidget::onRequestResource()
{
    if (!m_banker) {
        QMessageBox::warning(this, "未初始化", "请先初始化系统！");
        return;
    }

    int pid = m_requestPidSpin->value();
    if (pid < 0 || pid >= m_processCount) {
        QMessageBox::warning(this, "参数错误",
                             QString("进程号必须在 0~%1 之间！").arg(m_processCount - 1));
        return;
    }

    std::vector<int> request = parseResourceInput(
        m_requestInput->text(), m_resourceCount);

    log(QString("→ 进程 P%1 申请资源: %2")
            .arg(pid).arg(m_requestInput->text()));

    std::vector<int> safeSequence;
    bool granted = m_banker->requestResources(pid, request, safeSequence);

    if (granted) {
        QString seqStr;
        for (size_t k = 0; k < safeSequence.size(); ++k) {
            seqStr += QString("P%1").arg(safeSequence[k]);
            if (k < safeSequence.size() - 1) seqStr += " → ";
        }
        log("✅ 安全性检查通过，资源分配成功！");
        log(QString("   安全序列: %1").arg(seqStr));
    } else {
        log("❌ 系统不安全，资源申请被拒绝！状态已回滚。");
    }

    refreshStatusDisplay();
}

void DeviceWidget::refreshStatusDisplay()
{
    if (!m_banker) return;

    // 更新 ALLOCATION 表
    const auto& alloc = m_banker->getAllocation();
    m_allocTable->setRowCount(m_processCount);
    m_allocTable->setColumnCount(m_resourceCount);
    QStringList headers;
    for (int j = 0; j < m_resourceCount; ++j)
        headers << QString("资源%1").arg(j);
    m_allocTable->setHorizontalHeaderLabels(headers);
    QStringList rowHeaders;
    for (int i = 0; i < m_processCount; ++i)
        rowHeaders << QString("P%1").arg(i);
    m_allocTable->setVerticalHeaderLabels(rowHeaders);

    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            int val = (i < static_cast<int>(alloc.size())
                       && j < static_cast<int>(alloc[i].size()))
                          ? alloc[i][j] : 0;
            m_allocTable->setItem(i, j, new QTableWidgetItem(QString::number(val)));
        }
    }

    // 更新 NEED 表
    const auto& need = m_banker->getNeed();
    m_needTable->setRowCount(m_processCount);
    m_needTable->setColumnCount(m_resourceCount);
    m_needTable->setHorizontalHeaderLabels(headers);
    m_needTable->setVerticalHeaderLabels(rowHeaders);

    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            int val = (i < static_cast<int>(need.size())
                       && j < static_cast<int>(need[i].size()))
                          ? need[i][j] : 0;
            m_needTable->setItem(i, j, new QTableWidgetItem(QString::number(val)));
        }
    }

    // 更新可用资源显示
    const auto& avail = m_banker->getAvailable();
    QStringList parts;
    for (int v : avail) parts << QString::number(v);
    m_availableInput->setText(parts.join(" "));
}

void DeviceWidget::log(const QString& message)
{
    m_logEdit->appendPlainText(message);
}

std::vector<int> DeviceWidget::parseResourceInput(const QString& text, int expectedCount)
{
    std::vector<int> result(expectedCount, 0);
    QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    for (int j = 0; j < expectedCount && j < parts.size(); ++j) {
        result[j] = parts[j].toInt();
    }
    return result;
}
