#ifndef FILE_WIDGET_H
#define FILE_WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

class QLineEdit;
class QTreeWidget;
class QPlainTextEdit;

#include "file_system.h"

// 文件系统模拟界面
class FileWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

private slots:
    // 处理用户输入的命令
    void onCommandEntered();

private:
    void setupUI();
    // 刷新目录树显示
    void refreshTree();
    // 追加输出到终端
    void appendOutput(const QString& text);
    // 递归构建树节点
    void buildTreeNode(QTreeWidgetItem* parent_item, class FileNode* node);

    //======界面控件======
    QLineEdit* path_display_;     // 当前路径显示
    QTreeWidget* tree_widget_;    // 目录树
    QPlainTextEdit* terminal_;    // 命令终端
    QLineEdit* command_input_;    // 命令输入框

    FileSystem fs_;               // 文件系统实例
};

#endif // FILE_WIDGET_H
