#pragma once

#include <algorithm>
#include <unordered_map>
#include<unordered_set>
#include <jsoncpp/json/json.h>
#include "tool.hpp"
#include "index.hpp"
#include"log.hpp"
namespace ns_searcher
{
    class Searcher
    {
    private:
        ns_index::Index *index;
        struct IntegrateInfo
        {
            uint64_t doc_id;
            int allweight;
            std::unordered_set<std::string> words;
            IntegrateInfo() : allweight(0), doc_id(0) {}
        };

    public:
        Searcher() {};
        ~Searcher() {};

    public:
        void InitSearcher(const std::string &path)
        {
            // 1.获取或创建index对象
            index = ns_index::Index::GetInstance();
            LOG(1,"成功获取index单例");
            // 2.根据index对象建立索引
            index->BulidIndex(path);
            LOG(1,"成功建立正排和倒排索引");
        }
        // query:用户搜索的东西
        // json_string:返回给用户的搜索结果
        void Search(const std::string &query, std::string *json_string)//不会有重复的跳转链接
        {
            // 1.分词：对query按照searcher的要求进行分词
            std::vector<std::string> words;
            ns_tool::JiebaUtil::CutString(query, &words);
            // 2.触发：根据分词的各个词，进行index查找
            std::unordered_map<uint64_t, IntegrateInfo> besingle;
            for (std::string s : words)
            {
                boost::to_lower(s);
                ns_index::InvertedList *tmp = index->GetInvertedList(s);
                if (nullptr == tmp)
                    continue;
                for(const ns_index::InvertedIndexInfo e:(*tmp))
                {
                    IntegrateInfo& integ=besingle[e.doc_id];
                    integ.doc_id=e.doc_id;
                    integ.allweight+=e.weight;
                    integ.words.insert(e.word);
                }
            }
            std::vector<std::pair<uint64_t, IntegrateInfo>>forsort(besingle.begin(),besingle.end());
            // 3.合并排序：汇总查找结果，按照相关性降序排序
            std::sort(forsort.begin(), forsort.end(), [](const std::pair<uint64_t, IntegrateInfo> &e1, const std::pair<uint64_t, IntegrateInfo> &e2)
                      { return e1.second.allweight>e2.second.allweight; });
            // 4.构建：根据查找出来的结果，构建json串
            Json::Value root;
            for (auto &e : forsort)
            {
                ns_index::ForwardIndexInfo *doc = index->GetFileInfo(e.second.doc_id);
                if (doc == nullptr)
                    continue;
                Json::Value elem;
                elem["title"] = doc->title;
                auto it=e.second.words.begin();
                elem["desc"] = GetDesc(doc->content, *it);
                elem["url"] = doc->url;
                root.append(elem);
            }
            Json::FastWriter writer;
            *json_string = writer.write(root);
        }
        // void Search(const std::string&query,std::string*json_string)//搜出来会有重复跳转链接
        // {
        //     //1.分词：对query按照searcher的要求进行分词
        //     std::vector<std::string>words;
        //     ns_tool::JiebaUtil::CutString(query,&words);
        //     //2.触发：根据分词的各个词，进行index查找
        //     ns_index::InvertedList IL;//这个拉链是很多拉链的结合，可能不同的词指向了同一个文件，这是一个问题
        //     for(std::string s:words)
        //     {
        //         boost::to_lower(s);
        //         ns_index::InvertedList* tmp=index->GetInvertedList(s);
        //         if(nullptr==tmp)continue;
        //         IL.insert(IL.end(),tmp->begin(),tmp->end());
        //     }
        //     //3.合并排序：汇总查找结果，按照相关性降序排序
        //     std::sort(IL.begin(),IL.end(),[](const ns_index::InvertedIndexInfo&e1,const ns_index::InvertedIndexInfo&e2){
        //         return e1.weight>e2.weight;
        //     });
        //     //4.构建：根据查找出来的结果，构建json串
        //     Json::Value root;
        //     for(auto&e:IL)
        //     {
        //         ns_index::ForwardIndexInfo*doc=index->GetFileInfo(e.doc_id);
        //         if(doc==nullptr)continue;
        //         Json::Value elem;
        //         elem["title"]=doc->title;
        //         elem["desc"]=GetDesc(doc->content,e.word);
        //         elem["url"]=doc->url;
        //         root.append(elem);
        //     }
        //     Json::StyledWriter writer;
        //     *json_string=writer.write(root);
        // }
        std::string GetDesc(const std::string &content, const std::string &key)
        {
            // 截取这个词前面50字节和后面100字节的内容，如果不够，到头即可
            const std::size_t prev_step = 50;
            const std::size_t next_step = 100;
            auto iter = std::search(content.begin(), content.end(), key.begin(), key.end(), [](int x, int y)
                                    {
                                        return (std::tolower(x) == std::tolower(y)); // 均转成小写比较
                                    });
            if (iter == content.end())
            {
                return "None1";
            }
            std::size_t pos = std::distance(content.begin(), iter); // 求iter的位置
            std::size_t start = 0;
            std::size_t end = content.size() - 1; // size_t要注意范围越界
            if (pos > start + prev_step)
                start = pos - prev_step;
            if (pos + next_step < end)
                end = pos + next_step;
            if (start >= end)
                return "None2";
            return content.substr(start, end - start) + "...";
        }
    };
}