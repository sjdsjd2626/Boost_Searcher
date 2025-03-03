#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include "tool.hpp"
#include"log.hpp"
namespace ns_index
{
    typedef struct ForwardIndexInfo // 正排索引信息，通过文档ID找到文档的一系列内容
    {
        std::string title;
        std::string content;
        std::string url;
        u_int64_t doc_id; // 文档的ID
    } ForwardIndexInfo;
    typedef struct InvertedIndexInfo // 倒排索引信息，通过关键字找到文档ID
    {
        u_int64_t doc_id;
        std::string word;
        int weight; // 权重
    } InvertedIndexInfo;

    typedef std::vector<InvertedIndexInfo> InvertedList;
    class Index
    {
    private:
        // 数组的下标天然作为文档的ID
        std::vector<ForwardIndexInfo> forward_index;
        // 倒排索引是关键字和倒排拉链的映射关系
        std::unordered_map<std::string, InvertedList> inverted_index;

    private:
        Index() {}
        Index(const Index &) = delete;
        Index &operator=(const Index &) = delete;
        static Index *instance;
        static std::mutex mtx;

    public:
        ~Index() {}
        static Index *GetInstance() // 懒汉模式
        {
            if (instance == nullptr)
            {
                mtx.lock();
                if (instance == nullptr)
                {
                    instance = new Index();
                }
                mtx.unlock();
            }
            return instance;
        }

    public:
        // 根据文档ID找到文档信息
        ForwardIndexInfo *GetFileInfo(u_int64_t doc_id)
        {
            if (doc_id >= forward_index.size())
            {
                LOG(4,"doc_id out range,error!!!");
                return nullptr;
            }
            return &forward_index[doc_id];
        }
        // 根据关键字，获得倒排拉链
        InvertedList *GetInvertedList(const std::string &word)
        {
            auto it = inverted_index.find(word);
            if (it == inverted_index.end())
            {
                LOG(2,word + " have no InvertedList");
                return nullptr;
            }
            return &(it->second);
        }
        // 根据data/output/raw.txt中的内容，构建正排和倒排索引
        bool BulidIndex(const std::string &path)
        {
            std::ifstream in(path, std::ios::in | std::ios::binary);
            if (!in.is_open())
            {
                LOG(4,path + " open error");
                return false;
            }
            std::string line;
            int count=0;
            while (std::getline(in, line))
            {
                ForwardIndexInfo *doc = BulidForwardIndex(line);
                if (nullptr == doc)
                {
                    LOG(4,"BulidForwardIndex error");
                    continue;
                }
                BulidInvertedIndex(*doc);
                count++;
                if(count%50==0)
                LOG(1,"已经建立："+std::to_string(count)+" 组索引");
            }
            return true;
        }

    private:
        ForwardIndexInfo *BulidForwardIndex(const std::string &line)
        {
            std::vector<std::string> results;
            const std::string sep(1, '\3');
            // 切分字符串
            ns_tool::CutString(line, &results, sep);
            if (results.size() != 3)
                return nullptr;
            // 构建对象
            ForwardIndexInfo doc;
            doc.title = results[0];
            doc.content = results[1];
            doc.url = results[2];
            doc.doc_id = forward_index.size();
            // 插入到正排索引vector中
            forward_index.push_back(std::move(doc));
            return &forward_index.back();
        }
        bool BulidInvertedIndex(const ForwardIndexInfo &doc)
        {
            struct word_cnt // 记录在标题和内容中出现的次数
            {
                int title_cnt;
                int content_cnt;
                word_cnt() : title_cnt(0), content_cnt(0) {}
            };
            std::unordered_map<std::string, word_cnt> word_cnt_map; // 关键字和次数的映射
            std::vector<std::string> title_words;                   // 下面均是统计次数
            ns_tool::JiebaUtil::CutString(doc.title, &title_words);
            for (std::string s : title_words)
            {
                boost::to_lower(s); // 搜索时要忽略大小写
                word_cnt_map[s].title_cnt++;
            }
            std::vector<std::string> content_words;
            ns_tool::JiebaUtil::CutString(doc.content, &content_words);
            for (std::string s : content_words)
            {
                boost::to_lower(s);
                word_cnt_map[s].content_cnt++;
            }
#define X 10
#define Y 1
            for (auto &pair : word_cnt_map)
            {
                InvertedIndexInfo item;
                item.doc_id = doc.doc_id;
                item.word = pair.first;
                item.weight = X * pair.second.title_cnt + Y * pair.second.content_cnt;
                InvertedList &tmp = inverted_index[pair.first];
                tmp.push_back(item); // 向倒排拉链中插入
            }
            return true;
        }
    };
    Index *Index::instance = nullptr;
    std::mutex Index::mtx;
}