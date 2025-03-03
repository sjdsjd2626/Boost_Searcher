#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "log.hpp"
#include "tool.hpp"
const std::string data_path = "data/input";
const std::string handle_path = "data/output/raw.txt";

typedef struct DocInfo
{
    std::string _title;
    std::string _content;
    std::string _url;
} DocInfo;

bool EnumFile(const std::string &srcpath, std::vector<std::string> *out);
bool ParseFile(const std::vector<std::string> &path_list, std::vector<DocInfo> *info);
bool SaveFile(const std::vector<DocInfo> &info, const std::string &path);

int main()
{
    std::vector<std::string> files_path_list;
    // 把每一个文件的路径（包含文件名）都放到这个vector中
    if (!EnumFile(data_path, &files_path_list))
    {
        LOG(4,"enum file error");
        return 1;
    }
    std::vector<DocInfo> files_info;
    // 把每个文件中的内容解析拆分成标题、内容和url后放到这个vector中
    if (!ParseFile(files_path_list, &files_info))
    {
        LOG(4,"parse file error");
        return 2;
    }
    // 把拆分完的结果放到raw.txt文件中，方便后面的使用，格式是title\3content\3url \n title\3content\3url \n title\3content\3url \n......
    if (!SaveFile(files_info, handle_path)) // 方便getline直接读取一行
    {
        LOG(4,"save file error");
        return 3;
    }
    return 0;
}

// const & 输入
// * 输出
// & 输入输出
bool EnumFile(const std::string &srcpath, std::vector<std::string> *out)
{
    namespace fs = boost::filesystem;
    // 创建一个叫做root_path的对象
    fs::path root_path(srcpath);
    // 如果说路径不存在，就错误返回
    if (!fs::exists(root_path))
    {
        LOG(4,srcpath+" not exists");
        return false;
    }
    // 定义一个空的迭代器，用来进行判断递归结束
    fs::recursive_directory_iterator end;
    // 迭代器式遍历每个文件
    for (fs::recursive_directory_iterator iter(root_path); iter != end; iter++)
    {
        if (!fs::is_regular_file(*iter)) // 不是一个常规文件（是一个目录）的话,继续下一个
        {
            continue;
        }
        if (iter->path().extension() != ".html") // 后缀不为html的话,继续下一个
        {
            continue;
        }
        out->push_back(iter->path().string()); // 存储起来
    }
    return true;
}
// 这里的static影响的是链接属性，表明此函数仅能在定义它的文件内部使用
static bool ExtractTitle(const std::string &res, std::string *out)
{
    size_t begin = res.find("<title>");
    if (begin == std::string::npos)
        return false;
    size_t end = res.find("</title>");
    if (end == std::string::npos)
        return false;
    begin += std::string("<title>").size();
    if (begin > end)
        return false;
    *out = res.substr(begin, end - begin);
    return true;
}
static bool ExtractContent(const std::string &res, std::string *out)
{ // 去标签化，基于一个简单的状态机
    enum status
    {
        LABLE,
        CONTENT
    };
    enum status s = LABLE;
    for (char c : res)
    {
        switch (s)
        {
        case LABLE:
            if (c == '>')
                s = CONTENT;
            break;
        case CONTENT:
            if (c == '<')
                s = LABLE;
            else
            {
                if (c == '\n') // 不用\n，把内容放在一行
                    c = ' ';
                out->push_back(c);
            }
            break;
        default:
            break;
        }
    }
    return true;
}
static bool ConstructUrl(const std::string &path, std::string *out)
{
    std::string url_head = "https://www.boost.org/doc/libs/1_87_0/doc/html";
    std::string url_tail = path.substr(data_path.size());
    *out = url_head + url_tail;
    return true;
}
bool ParseFile(const std::vector<std::string> &path_list, std::vector<DocInfo> *info)
{
    for (const std::string &tmp : path_list)
    {
        // 读取文件内容，放到result中
        std::string result;
        if (!ns_tool::ReadFile(tmp, &result))
        {
            continue;
        }
        DocInfo doc;
        // 提取文件内容的title
        if (!ExtractTitle(result, &doc._title))
        {
            continue;
        }
        // 提取文件内容的content
        if (!ExtractContent(result, &doc._content))
        {
            continue;
        }
        // 构建.html文件对应的官网的url
        if (!ConstructUrl(tmp, &doc._url))
        {
            continue;
        }
        info->push_back(std::move(doc));
    }
    return true;
}
bool SaveFile(const std::vector<DocInfo> &info, const std::string &path)
{
    std::ofstream out(path, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        LOG(4,"open " + path + " failed");
        return false;
    }
    for (const DocInfo &tmp : info)
    {
        std::string out_string;
        out_string += tmp._title;
        out_string += '\3';
        out_string += tmp._content;
        out_string += '\3';
        out_string += tmp._url;
        out_string += '\n';
        out.write(out_string.c_str(), out_string.size());
    }
    return true;
}