#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <QString>
#include <memory>
#include <vector>

// 文件/目录节点
struct FileNode {
    QString name;
    bool is_directory = false;
    QString content;
    FileNode* parent = nullptr;
    std::vector<std::unique_ptr<FileNode>> children;
};

// 模拟文件系统
class FileSystem {
public:
    FileSystem();

    // 执行一条命令，返回输出文本
    QString execute(const QString& command);

    // 当前工作目录路径
    QString currentPath() const;

    // 获取根节点（供 UI 层构建目录树）
    FileNode* rootNode() const;

    // 获取当前工作目录节点
    FileNode* currentNode() const;

private:
    std::unique_ptr<FileNode> root_;  // 根节点
    FileNode* current_dir_;           // 当前工作目录

    // 初始化预设文件树
    void initFileTree();

    //======命令实现======
    QString cmd_cd(const QString& arg);
    QString cmd_dir(const QString& arg);
    QString cmd_md(const QString& arg);
    QString cmd_rd(const QString& arg);
    QString cmd_edit(const QString& arg);
    QString cmd_del(const QString& arg);

    //======辅助方法======
    // 在节点的 children 中查找目录
    FileNode* findDir(FileNode* parent, const QString& name) const;
    // 在节点的 children 中查找文件
    FileNode* findFile(FileNode* parent, const QString& name) const;
    // 构建路径字符串
    QString buildPath(FileNode* node) const;
};

#endif // FILE_SYSTEM_H
