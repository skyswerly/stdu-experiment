#ifndef DEVICE_WIDGET_H
#define DEVICE_WIDGET_H

#include <QWidget>
#include <vector>
#include <memory>

class QSpinBox;
class QTableWidget;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QLineEdit;
class IBanker;

class DeviceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceWidget(QWidget* parent = nullptr);
    ~DeviceWidget() override;

private slots:
    void onInitialize();
    void onRequestResource();
    void refreshStatusDisplay();

private:
    void setupUI();
    void log(const QString& message);
    std::vector<int> parseResourceInput(const QString& text, int expectedCount);

    // 业务对象
    std::unique_ptr<IBanker> m_banker;

    // 参数控件
    QSpinBox* m_processCountSpin;
    QSpinBox* m_resourceCountSpin;
    QPushButton* m_initBtn;

    // 可用资源输入
    QLabel* m_availableLabel;
    QLineEdit* m_availableInput;

    // MAX 矩阵编辑
    QLabel* m_maxLabel;
    QTableWidget* m_maxTable;

    // 资源申请
    QSpinBox* m_requestPidSpin;
    QLineEdit* m_requestInput;
    QPushButton* m_requestBtn;

    // 状态显示
    QTableWidget* m_allocTable;
    QTableWidget* m_needTable;

    // 日志
    QPlainTextEdit* m_logEdit;

    // 状态
    int m_processCount = 5;
    int m_resourceCount = 3;
};

#endif // DEVICE_WIDGET_H
