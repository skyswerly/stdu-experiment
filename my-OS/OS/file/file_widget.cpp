#include "file_widget.h"
#include "file_system.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

FileWidget::FileWidget(QWidget* parent)
    : QWidget(parent) {
    setupUI();
    refreshTree();
    appendOutput("模拟文件系统已启动。输入 'dir' 查看根目录内容。");
}

void FileWidget::setupUI() {
    auto* main_layout = new QVBoxLayout(this);

    // ====== 当前路径 ======
    auto* path_layout = new QHBoxLayout();
    path_layout->addWidget(new QLabel("当前路径:", this));
    path_display_ = new QLineEdit(this);
    path_display_->setReadOnly(true);
    path_display_->setText(fs_.currentPath());
    path_layout->addWidget(path_display_);
    main_layout->addLayout(path_layout);

    // ====== 左右分栏：目录树 | 终端 ======
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    tree_widget_ = new QTreeWidget(this);
    tree_widget_->setHeaderLabel("目录结构");
    splitter->addWidget(tree_widget_);

    terminal_ = new QPlainTextEdit(this);
    terminal_->setReadOnly(true);
    splitter->addWidget(terminal_);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    main_layout->addWidget(splitter);

    // ====== 命令输入 ======
    auto* cmd_layout = new QHBoxLayout();
    cmd_layout->addWidget(new QLabel(">", this));
    command_input_ = new QLineEdit(this);
    command_input_->setPlaceholderText("输入命令 (cd/dir/md/rd/edit/del/exit)");
    cmd_layout->addWidget(command_input_);
    main_layout->addLayout(cmd_layout);

    connect(command_input_, &QLineEdit::returnPressed,
            this, &FileWidget::onCommandEntered);
}

void FileWidget::onCommandEntered() {
    QString cmd = command_input_->text().trimmed();
    if (cmd.isEmpty()) return;

    // exit 特殊处理
    if (cmd.toLower() == "exit") {
        qApp->quit();
        return;
    }

    appendOutput("> " + cmd);

    QString output = fs_.execute(cmd);
    if (!output.isEmpty()) {
        appendOutput(output);
    }

    // 更新视图
    path_display_->setText(fs_.currentPath());
    refreshTree();
    command_input_->clear();
}

void FileWidget::refreshTree() {
    tree_widget_->clear();
    FileNode* root = fs_.rootNode();
    if (!root) return;

    auto* root_item = new QTreeWidgetItem(tree_widget_);
    root_item->setText(0, "/");
    root_item->setExpanded(true);

    for (auto& child : root->children) {
        buildTreeNode(root_item, child.get());
    }

    // 简单起见全部展开
    tree_widget_->expandAll();
}

void FileWidget::buildTreeNode(QTreeWidgetItem* parent_item, FileNode* node) {
    auto* item = new QTreeWidgetItem(parent_item);
    QString label = node->is_directory ? node->name + "/" : node->name;
    item->setText(0, label);

    for (auto& child : node->children) {
        buildTreeNode(item, child.get());
    }
}

void FileWidget::appendOutput(const QString& text) {
    terminal_->appendPlainText(text);
}
