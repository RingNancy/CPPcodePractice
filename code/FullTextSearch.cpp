#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <set>

std::mutex mtx;
std::unordered_map<std::string, int> invertedIndex;
std::unordered_map<std::string, double> timeCosts;  // 用于存储每个关键词的查找时间

// 停用词集合，使用 unordered_set 来提高查找效率
std::unordered_set<std::string> stopWords = {
    "a", "the", "and", "is", "are", "be", "to", "of", "in", "it", "on", "for", "with", "as", "this", "by", "at"
};

bool isStopWord(const std::string &word) {
    return stopWords.find(word) != stopWords.end();
}

void searchInFileChunk(std::ifstream &dataFile, size_t chunkSize, const std::vector<std::string> &keywords) {
    std::unordered_map<std::string, int> localIndex;
    std::unordered_map<std::string, double> localTimeCosts;

    std::string chunk;
    chunk.resize(chunkSize);

    // 读取文件块并进行搜索
    while (dataFile.read(&chunk[0], chunkSize) || dataFile.gcount() > 0) {
        size_t bytesRead = dataFile.gcount();
        chunk.resize(bytesRead);  // 调整为实际读取的大小

        for (const auto &keyword : keywords) {
            if (isStopWord(keyword)) {
                continue;  // 跳过停用词
            }

            auto start = std::chrono::high_resolution_clock::now();

            size_t pos = 0;
            while ((pos = chunk.find(keyword, pos)) != std::string::npos) {
                localIndex[keyword]++;
                pos += keyword.length();
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            localTimeCosts[keyword] = elapsed.count();  // 记录该关键词的查找时间
        }
    }

    // 将局部结果合并到全局结果
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &pair : localIndex) {
        invertedIndex[pair.first] += pair.second;
    }
    for (const auto &pair : localTimeCosts) {
        timeCosts[pair.first] += pair.second;  // 合并时间
    }
}

int main() {
    std::ifstream inputFile("../wpsTrainWork/keyword.txt");
    std::ifstream dataFile("../wpsTrainWork/enwiki-20231120-abstract1.xml");
    std::ofstream outputFile("../wpsTrainWork/output02.txt");

    // 读取关键词列表
    std::vector<std::string> keywords;
    std::string line;
    while (std::getline(inputFile, line)) {
        keywords.push_back(line);
        invertedIndex[line] = 0;
        timeCosts[line] = 0.0;
    }

    // 设置文件块大小
    size_t chunkSize = 2 * 1024 * 1024;  // 2MB
    std::vector<std::thread> threads;

    // 启动多个线程处理文件块
    while (dataFile.peek() != EOF) {
        threads.emplace_back(searchInFileChunk, std::ref(dataFile), chunkSize, std::ref(keywords));
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        t.join();
    }

    // 输出结果
    for (const auto &pair : invertedIndex) {
        outputFile << pair.first << ": count=" << pair.second
                   << ", time_cost=" << timeCosts[pair.first] * 1000 << " ms\n";
    }

    return 0;
}