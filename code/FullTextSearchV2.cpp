#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <future>
#include <chrono>
#include <mutex>
#include <string>

// 停用词集合
std::unordered_set<std::string> stopWords = {
    "a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "aren't", 
    "as", "at", "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", 
    "can't", "cannot", "could", "couldn't", "did", "didn't", "do", "does", "doesn't", "don't", "doing", 
    "each", "few", "for", "from", "further", "had", "hadn't", "has", "hasn't", "haven't", "have", "haven't", 
    "he", "he'd", "he'll", "he's", "here", "how", "how's", "how", "how'll", "etc"
};

// 检查是否为停用词
bool isStopWord(const std::string &word) {
    return stopWords.find(word) != stopWords.end();
}

// Trie 树节点
struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord;
    
    TrieNode() : isEndOfWord(false) {}

    ~TrieNode()
    {
        for (auto& pair : children)
        {
            delete pair.second;
        }
    }
};

// 插入单词到 Trie 树
void insertToTrie(TrieNode* root, const std::string& word) {
    TrieNode* node = root;
    for (char ch : word) {
        if (node->children.find(ch) == node->children.end()) {
            node->children[ch] = new TrieNode();
        }
        node = node->children[ch];
    }
    node->isEndOfWord = true;
}

// 使用 Trie 树进行匹配
bool searchInTrie(TrieNode* root, const std::string& word) {
    TrieNode* node = root;
    for (char ch : word) {
        if (node->children.find(ch) == node->children.end()) {
            return false;
        }
        node = node->children[ch];
    }
    return node->isEndOfWord;
}

// 反向索引：关键词 -> 关键词出现次数
std::unordered_map<std::string, size_t> keywordCount;
std::unordered_map<std::string, std::vector<size_t>> keywordPositions;  // 保存匹配位置
std::mutex countMutex;  // 互斥锁，保护对 keywordCount 和 keywordPositions 的访问

// 搜索文件块
void searchInFileChunk(const std::string &chunk, size_t offset, TrieNode* trieRoot, const std::vector<std::string> &keywords) {
    for (const auto &keyword : keywords) {
        size_t pos = 0;
        while ((pos = chunk.find(keyword, pos)) != std::string::npos) {
            std::lock_guard<std::mutex> guard(countMutex);  // 锁定互斥锁，保护 keywordCount 和 keywordPositions
            keywordCount[keyword]++;  // 统计出现次数
            keywordPositions[keyword].push_back(offset + pos);  // 保存匹配位置
            pos += keyword.length();
        }
    }
}

int main() {
    std::ifstream inputFile("../wpsTrainWork/keyword.txt");
    std::ifstream dataFile("../wpsTrainWork/enwiki-20231120-abstract1.xml");
    std::ofstream outputFile("../wpsTrainWork/output02.txt");

    // 读取关键词列表并过滤停用词
    std::vector<std::string> keywords;
    TrieNode* trieRoot = new TrieNode();
    std::string line;

    while (std::getline(inputFile, line)) {
        if (!isStopWord(line)) {
            keywords.push_back(line);
            insertToTrie(trieRoot, line);  // 将关键词插入 Trie 树
        }
    }

    // 设置文件块大小
    size_t chunkSize = 16 * 1024 * 1024;  // 16MB
    size_t overlapSize = 32;  // 重叠区域大小，确保跨块的关键词能匹配
    size_t currentOffset = 0;  // 当前块的偏移量
    std::vector<std::future<void>> futures;
    std::string previousChunkTail;  // 记录上一个块的尾部

    // 读取文件并分块处理
    while (dataFile.peek() != EOF) {
        std::string chunk;
        chunk.resize(chunkSize + overlapSize);  // 增加重叠区域大小

        // 读取数据块
        dataFile.read(&chunk[overlapSize], chunkSize);  // 从重叠区域后开始读取
        size_t bytesRead = dataFile.gcount();
        if (bytesRead == 0) break;

        // 截断块的大小为实际读取的字节数
        chunk.resize(bytesRead + overlapSize);

        // 拼接上一个块的尾部到当前块的头部
        chunk.replace(0, overlapSize, previousChunkTail);

        // 更新当前块的尾部
        if (chunk.size() > overlapSize) {
            previousChunkTail = chunk.substr(chunk.size() - overlapSize);
        }

        // 提交异步任务
        futures.push_back(std::async(std::launch::async, searchInFileChunk, chunk, currentOffset, trieRoot, std::ref(keywords)));

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

    // 输出结果并计算时间
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (const auto &keyword : keywords) {
        auto keywordStartTime = std::chrono::high_resolution_clock::now();

        size_t count = keywordCount[keyword];  // 获取关键词出现次数
        std::chrono::duration<double> keywordDuration = std::chrono::high_resolution_clock::now() - keywordStartTime;

        outputFile << keyword << ": count=" << count << ", time_cost=" << keywordDuration.count() * 1000 << "ms\n"; // 输出时间为毫秒
    }

    // 释放 Trie 树的内存
    delete trieRoot;

    // 记录总耗时
    auto totalDuration = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Total time: " << std::chrono::duration<double>(totalDuration).count() * 1000 << " ms" << std::endl; // 总时间输出为毫秒

    return 0;
}
