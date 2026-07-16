#ifndef ADDRESS_GENERATOR_H
#define ADDRESS_GENERATOR_H

#include <vector>

// 指令逻辑地址生成器
class AddressGenerator {
public:
    // 无参构造函数
    AddressGenerator();

    // 自动生成 count 条指令地址
    std::vector<int> autoGenerate(int count = 20);

    // 获取指令地址序列
    const std::vector<int>& getAddresses() const;

    // 计算地址对应的虚拟页号（根据页面大小计算）
    std::vector<int> getPages(int pageSize) const;

    // 判断指令序列是否为空
    bool isEmpty() const;

private:
    std::vector<int> m_addresses;  // 逻辑地址序列
};

#endif // ADDRESS_GENERATOR_H
