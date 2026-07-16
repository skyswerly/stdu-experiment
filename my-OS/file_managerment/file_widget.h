#pragma once

#include <QWidget>
#include <QTreeWidgetItem>

class QLineEdit;
class QTreeWidget;
class QPlainTextEdit;

#include "file_system.h"

class FileWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

private slots:
    void onCommandEntered();

private:
    void setupUI();
    void refreshTree();
    void appendOutput(const QString& text);
    void buildTreeNode(QTreeWidgetItem* parent_item, class FileNode* node);

    QLineEdit* path_display_;
    QTreeWidget* tree_widget_;
    QPlainTextEdit* terminal_;
    QLineEdit* command_input_;

    FileSystem fs_;
};
