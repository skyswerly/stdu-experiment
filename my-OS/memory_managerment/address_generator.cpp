#include "address_generator.h"
#include <random>
#include <algorithm>
#include <iostream>

//无参构造函数
AddressGenerator::AddressGenerator() {
}

std::vector<int> AddressGenerator::autoGenerate(int count) {
    m_addresses.clear();

    int half    = count / 2;   // 50%：10 条顺序地址
    int quarter = count / 4;   // 25%： 5 条低地址 + 5 条高地址

    //随机数生成器
    std::mt19937 rng(std::random_device{}());

    // 起始基地址[0, 512]随机
    int base = std::uniform_int_distribution<int>(0, 512)(rng);
    int step = 256;  // 地址步长

    // ---- 第 1 段：50% 顺序连续地址 ----
    for (int i = 0; i < half; i++) {
        m_addresses.push_back(base + i);
        std::cout << base + i<<std::endl;
    }

    // 低地址区间上限 = 顺序段中点位置
    int midAddr = base + (half / 2) * step;

    // ---- 第 2 段：25% 低地址区间随机地址 ----
    // 区间：[0, midAddr)
    for (int i = 0; i < quarter; i++) {
        int lowAddr = std::uniform_int_distribution<int>(0, midAddr - 1)(rng);
        m_addresses.push_back(lowAddr);
    }

    // 高地址区间上限 = 基地址 + half * step * 2
    int highBound = base + half * step * 2;

    // ---- 第 3 段：25% 高地址区间随机地址 ----
    // 区间：[midAddr, highBound)
    for (int i = 0; i < quarter; i++) {
        int highAddr = std::uniform_int_distribution<int>(midAddr, highBound)(rng);
        m_addresses.push_back(highAddr);
    }

    // 随机打乱地址序列
    std::shuffle(m_addresses.begin(), m_addresses.end(), rng);

    return m_addresses;
}

//获取指令地址序列
const std::vector<int>& AddressGenerator::getAddresses() const {
    return m_addresses;
}

//计算地址对应的虚拟页号（根据页面大小计算）
std::vector<int> AddressGenerator::getPages(int pageSize) const {
    //为页号表定义并分配足够的空间
    std::vector<int> pages;
    pages.reserve(m_addresses.size());
    //计算虚拟页号
    for (int addr : m_addresses) {
        pages.push_back(addr / pageSize);
    }
    return pages;
}

//判断指令序列是否为空
bool AddressGenerator::isEmpty() const {
    return m_addresses.empty();
}
