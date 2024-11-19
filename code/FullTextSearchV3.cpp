#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <future>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>

// 停用词集合
const std::unordered_set<std::string> stopWords = {
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
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    bool isEndOfWord;

    TrieNode() : isEndOfWord(false) {}
};

// 插入单词到 Trie 树
void insertToTrie(TrieNode& root, const std::string& word) {
    TrieNode* node = &root;
    for (char ch : word) {
        if (!node->children[ch]) {
            node->children[ch] = std::make_unique<TrieNode>();
        }
        node = node->children[ch].get();
    }
    node->isEndOfWord = true;
}

// 使用 Trie 树进行多模式匹配 (Aho-Corasick 核心逻辑的简化实现)
void searchInTrie(const TrieNode& root, const std::string& text, size_t offset, 
                  std::unordered_map<std::string, size_t>& keywordCount, 
                  std::mutex& countMutex) {
    const TrieNode* node = &root;
    std::string matchedWord;

    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];

        while (node && !node->children.count(ch)) {
            node = nullptr;  // 重置为根节点，当前简化逻辑
        }

        if (node) {
            node = node->children.at(ch).get();
            matchedWord += ch;

            if (node->isEndOfWord) {
                std::lock_guard<std::mutex> guard(countMutex);  // 保护共享数据
                keywordCount[matchedWord]++;
            }
        } else {
            node = &root;
            matchedWord.clear();
        }
    }
}

// 读取文件块
std::vector<std::string> readFileChunks(std::ifstream& file, size_t chunkSize, size_t overlapSize, std::string& previousChunkTail) {
    std::vector<std::string> chunks;
    while (file.peek() != EOF) {
        std::string chunk;
        chunk.resize(chunkSize + overlapSize);

        file.read(&chunk[overlapSize], chunkSize);  // 从重叠区域后开始读取
        size_t bytesRead = file.gcount();
        if (bytesRead == 0) break;

        chunk.resize(bytesRead + overlapSize);  // 截断多余部分
        chunk.replace(0, overlapSize, previousChunkTail);  // 拼接前一个块的尾部
        previousChunkTail = chunk.substr(chunk.size() - overlapSize);

        chunks.push_back(chunk);
    }
    return chunks;
}

int main() {
    std::ifstream inputFile("../wpsTrainWork/keyword.txt");
    std::ifstream dataFile("../wpsTrainWork/enwiki-20231120-abstract1.xml");
    std::ofstream outputFile("../wpsTrainWork/output03.txt");

    if (!inputFile.is_open() || !dataFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Error: Failed to open one or more files." << std::endl;
        return 1;
    }

    // 读取关键词列表并过滤停用词
    TrieNode trieRoot;
    std::string line;
    while (std::getline(inputFile, line)) {
        if (!isStopWord(line)) {
            insertToTrie(trieRoot, line);  // 将关键词插入 Trie 树
        }
    }

    // 配置块大小与重叠区域
    size_t chunkSize = 16 * 1024 * 1024;  // 16MB
    size_t overlapSize = 32;  // 重叠区域大小
    std::string previousChunkTail;

    // 统计数据
    std::unordered_map<std::string, size_t> keywordCount;
    std::mutex countMutex;

    // 文件分块并并发处理
    auto startTime = std::chrono::high_resolution_clock::now();
    auto chunks = readFileChunks(dataFile, chunkSize, overlapSize, previousChunkTail);
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < chunks.size(); ++i) {
        futures.push_back(std::async(std::launch::async, searchInTrie, 
                                     std::ref(trieRoot), std::cref(chunks[i]), i * chunkSize, 
                                     std::ref(keywordCount), std::ref(countMutex)));
    }

    for (auto& f : futures) {
        f.get();
    }

    // 输出统计结果
    for (const auto& [keyword, count] : keywordCount) {
        auto keywordStartTime = std::chrono::high_resolution_clock::now();
        
        outputFile << keyword << ": count=" << count;
        
        // 记录处理时间（以毫秒为单位）
        auto keywordDuration = std::chrono::high_resolution_clock::now() - keywordStartTime;
        outputFile << ", time_cost=" << std::chrono::duration<double, std::milli>(keywordDuration).count() << "ms\n";
    }

    auto totalDuration = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Total time: " << std::chrono::duration<double, std::milli>(totalDuration).count() << " ms" << std::endl;

    return 0;
}
