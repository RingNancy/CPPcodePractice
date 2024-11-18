#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>

std::mutex mtx;
size_t totalCount = 0;
std::ofstream outputFile;

// 简单线性搜索
size_t linearSearch(const std::string& text, const std::string& keyword) {
    size_t count = 0;
    size_t pos = text.find(keyword);
    while (pos != std::string::npos) {
        ++count;
        pos = text.find(keyword, pos + keyword.size());
    }
    return count;
}

// Boyer-Moore 搜索算法
size_t boyerMooreSearchFast(const std::string& text, const std::string& keyword) {
    const size_t n = text.size();
    const size_t m = keyword.size();
    if (m > n) return 0;

    std::vector<int> badChar(256, -1);
    for (size_t i = 0; i < m; ++i) {
        badChar[(unsigned char)keyword[i]] = i;
    }

    size_t count = 0;
    size_t s = 0;
    while (s <= (n - m)) {
        size_t j = m - 1;
        while (j < m && keyword[j] == text[s + j]) {
            --j;
        }
        if (j == SIZE_MAX) {
            ++count;
            s += m;
        } else {
            s += std::max(1, (int)(j - badChar[(unsigned char)text[s + j]]));
        }
    }
    return count;
}

// 动态选择搜索方法
size_t search(const std::string& text, const std::string& keyword) {
    if (keyword.size() <= 5) {  // 小关键词使用线性搜索
        return linearSearch(text, keyword);
    } else {  // 长关键词使用 Boyer-Moore
        return boyerMooreSearchFast(text, keyword);
    }
}

// 搜索每个块并统计关键词次数
void searchInChunk(const std::string& filename, const std::string& keyword, size_t start, size_t end, size_t& localCount) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }

    size_t bufferSize = keyword.size() - 1;
    size_t adjustedStart = (start > bufferSize) ? start - bufferSize : 0;
    size_t adjustedEnd = end;

    file.seekg(adjustedStart);
    std::string buffer(adjustedEnd - adjustedStart, '\0');
    file.read(&buffer[0], adjustedEnd - adjustedStart);

    localCount = search(buffer, keyword);
}

// 搜索关键词
void searchKeyword(const std::string& filename, const std::string& keyword, size_t numThreads) {
    totalCount = 0;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }
    size_t fileSize = file.tellg();
    file.close();

    size_t chunkSize = fileSize / numThreads;
    std::vector<std::thread> threads;
    std::vector<size_t> localCounts(numThreads, 0);

    auto startTime = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;
        threads.emplace_back(searchInChunk, filename, keyword, start, end, std::ref(localCounts[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    for (const auto& count : localCounts) {
        totalCount += count;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    std::lock_guard<std::mutex> lock(mtx);
    outputFile << "keywords: '" << keyword << "' count: " << totalCount << ", timeCost: " << elapsed.count() << " ms\n";
    std::cout << "keywords: '" << keyword << "' count: " << totalCount << ", timeCost: " << elapsed.count() << " ms\n";
}

int main() {
    const std::string searchFilename = "../wpsTrainWork/enwiki-20231120-abstract1.xml";
    const std::string keywordsFilename = "../wpsTrainWork/keyword.txt";
    const std::string outputFilename = "../wpsTrainWork/output02.txt";
    const size_t numThreads = 16;

    std::ifstream keywordsFile(keywordsFilename);
    if (!keywordsFile) {
        std::cerr << "无法打开关键词文件: " << keywordsFilename << std::endl;
        return 1;
    }

    outputFile.open(outputFilename, std::ios::out | std::ios::trunc);
    if (!outputFile) {
        std::cerr << "无法创建输出文件: " << outputFilename << std::endl;
        return 1;
    }

    std::string keyword;
    while (std::getline(keywordsFile, keyword)) {
        if (!keyword.empty()) {
            searchKeyword(searchFilename, keyword, numThreads);
        }
    }

    keywordsFile.close();
    outputFile.close();
    system("pause");
    return 0;
}
