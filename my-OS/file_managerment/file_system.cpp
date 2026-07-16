#include "file_system.h"
#include <QStringList>

// ============================================================
// 构造 + 初始化
// ============================================================

FileSystem::FileSystem()
    : root_(std::make_unique<FileNode>()), current_dir_(nullptr) {
    root_->name = "/";
    root_->is_directory = true;
    root_->parent = nullptr;
    current_dir_ = root_.get();
    initFileTree();
}

void FileSystem::initFileTree() {
    // /
    // ├── home/
    // │   └── user/
    // ├── etc/
    // ├── var/
    // ├── boot/
    // └── README (文件)

    auto addDir = [this](FileNode* parent, const QString& name) -> FileNode* {
        auto node = std::make_unique<FileNode>();
        node->name = name;
        node->is_directory = true;
        node->parent = parent;
        FileNode* ptr = node.get();
        parent->children.push_back(std::move(node));
        return ptr;
    };

    auto addFile = [this](FileNode* parent, const QString& name, const QString& content = QString()) {
        auto node = std::make_unique<FileNode>();
        node->name = name;
        node->is_directory = false;
        node->content = content;
        node->parent = parent;
        parent->children.push_back(std::move(node));
    };

    FileNode* home = addDir(root_.get(), "home");
    addDir(home, "user");
    addDir(root_.get(), "etc");
    addDir(root_.get(), "var");
    addDir(root_.get(), "boot");
    addFile(root_.get(), "README", "Welcome to the simulated file system.");
}

// ============================================================
// 公共接口
// ============================================================

QString FileSystem::execute(const QString& command) {
    QString trimmed = command.trimmed();
    if (trimmed.isEmpty()) return QString();

    // 按空格分割命令和参数
    int space = trimmed.indexOf(' ');
    QString cmd = (space < 0) ? trimmed.toLower() : trimmed.left(space).toLower();
    QString arg = (space < 0) ? QString() : trimmed.mid(space + 1).trimmed();

    if (cmd == "cd")       return cmd_cd(arg);
    if (cmd == "dir")      return cmd_dir(arg);
    if (cmd == "md")       return cmd_md(arg);
    if (cmd == "rd")       return cmd_rd(arg);
    if (cmd == "edit")     return cmd_edit(arg);
    if (cmd == "del")      return cmd_del(arg);

    return QString("错误: 未知命令 '%1'").arg(cmd);
}

QString FileSystem::currentPath() const {
    return buildPath(current_dir_);
}

FileNode* FileSystem::rootNode() const {
    return root_.get();
}

FileNode* FileSystem::currentNode() const {
    return current_dir_;
}

// ============================================================
// 辅助方法
// ============================================================

FileNode* FileSystem::findDir(FileNode* parent, const QString& name) const {
    for (auto& child : parent->children) {
        if (child->is_directory && child->name == name)
            return child.get();
    }
    return nullptr;
}

FileNode* FileSystem::findFile(FileNode* parent, const QString& name) const {
    for (auto& child : parent->children) {
        if (!child->is_directory && child->name == name)
            return child.get();
    }
    return nullptr;
}

QString FileSystem::buildPath(FileNode* node) const {
    if (node == root_.get()) return "/";
    QString path;
    FileNode* cur = node;
    while (cur && cur != root_.get()) {
        path = "/" + cur->name + path;
        cur = cur->parent;
    }
    return path.isEmpty() ? "/" : path;
}

// ============================================================
// cd 命令
// ============================================================

QString FileSystem::cmd_cd(const QString& arg) {
    if (arg.isEmpty() || arg == "/") {
        current_dir_ = root_.get();
        return QString("当前路径: %1").arg(currentPath());
    }
    if (arg == "..") {
        if (current_dir_->parent) {
            current_dir_ = current_dir_->parent;
        }
        return QString("当前路径: %1").arg(currentPath());
    }

    FileNode* target = findDir(current_dir_, arg);
    if (target) {
        current_dir_ = target;
        return QString("当前路径: %1").arg(currentPath());
    }
    return QString("错误: 目录 '%1' 不存在").arg(arg);
}

// ============================================================
// dir 命令
// ============================================================

QString FileSystem::cmd_dir(const QString& arg) {
    FileNode* target = current_dir_;
    if (!arg.isEmpty()) {
        FileNode* sub = findDir(current_dir_, arg);
        if (sub) target = sub;
        else return QString("错误: 目录 '%1' 不存在").arg(arg);
    }

    QString out;
    out += QString("目录: %1\n").arg(buildPath(target));
    for (auto& child : target->children) {
        if (child->is_directory)
            out += QString("  <DIR>   %1/\n").arg(child->name);
        else
            out += QString("  <FILE>  %1\n").arg(child->name);
    }
    if (target->children.empty()) {
        out += "  (空)\n";
    }
    // 去掉末尾换行
    if (out.endsWith('\n')) out.chop(1);
    return out;
}

// ============================================================
// md / rd 命令
// ============================================================

QString FileSystem::cmd_md(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定目录名";

    if (findDir(current_dir_, arg)) {
        return QString("错误: '%1' 已存在").arg(arg);
    }

    auto node = std::make_unique<FileNode>();
    node->name = arg;
    node->is_directory = true;
    node->parent = current_dir_;
    current_dir_->children.push_back(std::move(node));
    return QString("目录 '%1' 创建成功").arg(arg);
}

QString FileSystem::cmd_rd(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定目录名";

    FileNode* target = findDir(current_dir_, arg);
    if (!target) {
        return QString("错误: 目录 '%1' 不存在").arg(arg);
    }
    if (!target->children.empty()) {
        return QString("错误: 目录 '%1' 非空，无法删除").arg(arg);
    }

    // 从 parent 的 children 中移除
    auto& siblings = current_dir_->children;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == target) {
            siblings.erase(it);
            break;
        }
    }
    return QString("目录 '%1' 已删除").arg(arg);
}

// ============================================================
// edit / del 命令
// ============================================================

QString FileSystem::cmd_edit(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定文件名";

    if (findFile(current_dir_, arg)) {
        return QString("错误: '%1' 已存在").arg(arg);
    }

    auto node = std::make_unique<FileNode>();
    node->name = arg;
    node->is_directory = false;
    node->parent = current_dir_;
    current_dir_->children.push_back(std::move(node));
    return QString("文件 '%1' 创建成功").arg(arg);
}

QString FileSystem::cmd_del(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定文件名";

    FileNode* target = findFile(current_dir_, arg);
    if (!target) {
        return QString("错误: 文件 '%1' 不存在").arg(arg);
    }

    auto& siblings = current_dir_->children;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == target) {
            siblings.erase(it);
            break;
        }
    }
    return QString("文件 '%1' 已删除").arg(arg);
}
