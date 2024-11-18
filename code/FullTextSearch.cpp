#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <future>
#include <chrono>
#include <mutex>

// 全局变量和互斥锁
std::mutex mtx;
std::unordered_map<std::string, int> invertedIndex;
std::unordered_map<std::string, double> timeCosts;
std::unordered_map<std::string, std::vector<size_t>> keywordPositions;  // 保存关键词出现的位置

// 停用词集合
std::unordered_set<std::string> stopWords = {
    "a", "the", "and", "is", "are", "be", "to", "of", "in", "it", "on", "for", "with", "as", "this", "by", "at"
};

// 检查是否为停用词
bool isStopWord(const std::string &word) {
    return stopWords.find(word) != stopWords.end();
}

// 搜索文件块
void searchInFileChunk(const std::string &chunk, size_t offset, const std::vector<std::string> &keywords) {
    std::unordered_map<std::string, int> localIndex;
    std::unordered_map<std::string, double> localTimeCosts;
    std::unordered_map<std::string, std::vector<size_t>> localPositions;

    for (const auto &keyword : keywords) {
        auto start = std::chrono::high_resolution_clock::now();

        size_t pos = 0;
        while ((pos = chunk.find(keyword, pos)) != std::string::npos) {
            localIndex[keyword]++;
            localPositions[keyword].push_back(offset + pos);  // 保存相对于文件开头的绝对位置
            pos += keyword.length();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        localTimeCosts[keyword] += elapsed.count();  // 累积搜索时间
    }

    // 合并到全局结果
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &pair : localIndex) {
        invertedIndex[pair.first] += pair.second;
    }
    for (const auto &pair : localTimeCosts) {
        timeCosts[pair.first] += pair.second;
    }
    for (const auto &pair : localPositions) {
        keywordPositions[pair.first].insert(keywordPositions[pair.first].end(), pair.second.begin(), pair.second.end());
    }
}

int main() {
    std::ifstream inputFile("../wpsTrainWork/keyword.txt");
    std::ifstream dataFile("../wpsTrainWork/enwiki-20231120-abstract1.xml");
    std::ofstream outputFile("../wpsTrainWork/output01.txt");

    // 读取关键词列表并过滤停用词
    std::vector<std::string> keywords;
    std::string line;
    while (std::getline(inputFile, line)) {
        if (!isStopWord(line)) {
            keywords.push_back(line);
            invertedIndex[line] = 0;
            timeCosts[line] = 0.0;
        }
    }

    // 设置文件块大小
    size_t chunkSize = 16 * 1024 * 1024;  // 10MB
    std::vector<std::future<void>> futures;

    // 读取文件并分块处理
    size_t currentOffset = 0;  // 当前块在文件中的偏移量
    while (dataFile.peek() != EOF) {
        std::string chunk;
        chunk.resize(chunkSize);

        // 读取一块数据
        dataFile.read(&chunk[0], chunkSize);
        size_t bytesRead = dataFile.gcount();
        chunk.resize(bytesRead);  // 调整块大小为实际读取大小

        // 提交异步任务
        futures.push_back(std::async(std::launch::async, searchInFileChunk, chunk, currentOffset, std::ref(keywords)));

        // 更新偏移量
        currentOffset += bytesRead;

        // 控制最大并发线程数量
        if (futures.size() >= std::thread::hardware_concurrency()) {
            for (auto &f : futures) {
                f.get();  // 等待任务完成
            }
            futures.clear();  // 清空已完成的任务
        }
    }

    // 确保所有任务完成
    for (auto &f : futures) {
        f.get();
    }

    // 输出结果
    for (const auto &pair : invertedIndex) {
        outputFile << pair.first << ": count=" << pair.second
                   << ", time_cost=" << timeCosts[pair.first] * 1000 << " ms\n";
    }

    // 可选：打印关键词位置
    // for (const auto &pair : keywordPositions) {
    //     std::cout << "Keyword: " << pair.first << ", Positions: ";
    //     for (size_t pos : pair.second) {
    //         std::cout << pos << " ";
    //     }
    //     std::cout << "\n";
    // }

    return 0;
}
