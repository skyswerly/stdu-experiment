#pragma once

#include <QString>
#include <memory>
#include <vector>

struct FileNode {
    QString name;
    bool is_directory = false;
    QString content;
    FileNode* parent = nullptr;
    std::vector<std::unique_ptr<FileNode>> children;
};

class FileSystem {
public:
    FileSystem();

    /// 执行一条命令，返回输出文本
    QString execute(const QString& command);

    /// 当前工作目录路径
    QString currentPath() const;

    /// 获取根节点（供 UI 层构建目录树）
    FileNode* rootNode() const;

    /// 获取当前工作目录节点
    FileNode* currentNode() const;

private:
    std::unique_ptr<FileNode> root_;
    FileNode* current_dir_;

    void initFileTree();

    // 命令实现
    QString cmd_cd(const QString& arg);
    QString cmd_dir(const QString& arg);
    QString cmd_md(const QString& arg);
    QString cmd_rd(const QString& arg);
    QString cmd_edit(const QString& arg);
    QString cmd_del(const QString& arg);

    // 辅助：在节点的 children 中查找目录
    FileNode* findDir(FileNode* parent, const QString& name) const;
    // 辅助：在节点的 children 中查找文件
    FileNode* findFile(FileNode* parent, const QString& name) const;
    // 辅助：构建路径字符串
    QString buildPath(FileNode* node) const;
};
