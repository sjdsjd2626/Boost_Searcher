#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "cppjieba/Jieba.hpp"
namespace ns_tool
{
    bool ReadFile(const std::string &path, std::string *result)
    {
        std::ifstream in(path, std::ios::in);
        if (!in.is_open())
        {
            std::cerr << "open file " << path << " error" << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(in, line)) // 通过重载，对象可以强制类型转换成bool类型
        {
            *result += line;
        }
        in.close();
        return true;
    }
    void CutString(const std::string &line, std::vector<std::string> *result, const std::string &sep)
    {
        boost::split(*result, line, boost::is_any_of(sep), boost::token_compress_on); // 最后一个参数是"/3/3/3/3/3"这种压缩成一个
    }
    const char *const DICT_PATH = "./dict/jieba.dict.utf8";
    const char *const HMM_PATH = "./dict/hmm_model.utf8";
    const char *const USER_DICT_PATH = "./dict/user.dict.utf8";
    const char *const IDF_PATH = "./dict/idf.utf8";
    const char *const STOP_WORD_PATH = "./dict/stop_words.utf8";

    class JiebaUtil
    {
    private:
        static cppjieba::Jieba jieba;

    public:
        static void CutString(const std::string &src, std::vector<std::string> *out)
        {
            jieba.CutForSearch(src, *out);
        }
    };
    cppjieba::Jieba JiebaUtil::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);

}